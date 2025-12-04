#include <SunSense_inferencing.h>
#include <Arduino.h>
#include "mbed.h"
#include <rtos.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <VL53L1X.h>
#include <cstring>   // for strcmp

using namespace mbed;
using namespace rtos;
using namespace std::chrono_literals;

// --- Pin Definitions ---
#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4 // DRV8825: LOW=enable, HIGH=disable
#define SLP_PIN 9    // LOW=disable, HIGH=enable

// --- tof ---
const uint8_t sensorCount = 4;

// Shared latest TOF readings (updated by tofThread, read by aiThread)
volatile uint16_t tofValues[sensorCount] = {0, 0, 0, 0};

// XSHUT pins
const uint8_t xshutPins[sensorCount] = { 5, 6, 7, 8 };

// --- motor out ---
DigitalOut stepPin(digitalPinToPinName(STEP_PIN));
DigitalOut dirPin (digitalPinToPinName(DIR_PIN));
DigitalOut enPin  (digitalPinToPinName(ENABLE_PIN));
DigitalOut slpPin (digitalPinToPinName(SLP_PIN));

// --- Sensor Objects ---
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
VL53L1X tof[sensorCount];

// --- Parameters ---
const int lightThreshold = 6;    // lux value to trigger movement (adjust as needed)
int bright = 1;
volatile bool motorActive = false;

// --- RTOS objects ---
Mutex printMutex; // guard Serial printing
Mutex motorMutex; // guard motor operations
Mutex tofMutex;   // guard tof readings

Thread luxThread;
Thread tofThread;
Thread aiThread;

void pulse(int microsec){
  stepPin = 1; wait_us(microsec);
  stepPin = 0; wait_us(microsec);
}

// motor moves
/*ex:
2 full rotations = 2072 steps
full rotation     = 1036 steps
half              = 518 steps
quarter           = 259 steps
*/
void moveSteps(float reps, bool dir, int freqHz){
  motorMutex.lock();
  motorActive = true;

  long steps = (long)(1036.0f * reps);

  // Wake motor driver
  slpPin = 1;           // awake
  dirPin = dir ? 1 : 0;
  enPin  = 0;           // enable driver

  {
    printMutex.lock();
    Serial.print("MOTOR : ");
    Serial.println(dir ? "Forward" : "Backward");
    printMutex.unlock();
  }

  int microsec = (int)(1e6 / (freqHz * 2.0f));
  for (long i = 0; i < steps; i++) pulse(microsec);

  enPin  = 1;           // disable outputs (no idle heat)
  slpPin = 0;           // sleep

  motorActive = false;
  motorMutex.unlock();
}

void tofInit(int freqHz){
  for (int i = 0; i < sensorCount; i++) {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }
  ThisThread::sleep_for(10ms);

  for (int i = 0; i < sensorCount; i++) {
    pinMode(xshutPins[i], INPUT);
    ThisThread::sleep_for(10ms);

    if (!tof[i].init()) {
      printMutex.lock();
      Serial.print("Failed to detect and initialize sensor ");
      Serial.println(i);
      printMutex.unlock();
      continue;
    }

    tof[i].setTimeout(50);
    tof[i].setDistanceMode(VL53L1X::Long);
    tof[i].setMeasurementTimingBudget(50000); // 50ms
    tof[i].setAddress(0x2A + i);
    tof[i].startContinuous(55); // 55ms between readings
  }
}

void luxInit(){
  if (!tsl.begin()) {
    printMutex.lock();
    Serial.println("SYSTEM: No TSL2561 found!");
    printMutex.unlock();
    return;
  }
  printMutex.lock();
  Serial.println("SYSTEM: TSL2561 initialized!");
  printMutex.unlock();

  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
}

void luxTask(){
  const float reps = 3;
  while (true) {
    sensors_event_t event;
    tsl.getEvent(&event);

    if (event.light && !isnan(event.light)) {
      printMutex.lock();
      Serial.print("LIGHT : ");
      Serial.print(event.light);
      Serial.println(" lux!");
      printMutex.unlock();

      if (event.light < lightThreshold && bright == 0) {
        printMutex.lock();
        Serial.println("LIGHT : Too dark -> rotating clockwise");
        printMutex.unlock();

        moveSteps(reps, true, 100);
        bright = 1;
      } else if (event.light > lightThreshold && bright == 1) {
        printMutex.lock();
        Serial.println("LIGHT : Bright enough -> rotating counterclockwise");
        printMutex.unlock();

        moveSteps(reps, false, 100);
        bright = 0;
      }
    } else {
      printMutex.lock();
      Serial.println("LIGHT : No light data");
      printMutex.unlock();
    }

    ThisThread::sleep_for(10000ms);
  }
}

void tofTask() {
  const uint32_t SAMPLE_INTERVAL_MS = 55; // ~18.18 Hz effective

  while (true) {
    if (motorActive) {
      ThisThread::sleep_for(5ms);
      continue;
    }

    uint16_t local[4] = {0, 0, 0, 0};

    for (uint8_t i = 0; i < sensorCount; i++) {
      int d = tof[i].read();
      bool timedOut = tof[i].timeoutOccurred();

      // if bad, reuse last known good reading
      if (timedOut || d <= 0) {
        d = tofValues[i];  // previous stable value
      }

      // clamp to sane physical range, but NOT to 500 magic
      if (d < 100)  d = 100;   // 10cm min
      if (d > 4000) d = 4000;  // 4m max

      local[i] = (uint16_t)d;
    }

    tofMutex.lock();
    for (uint8_t i = 0; i < sensorCount; i++) {
      tofValues[i] = local[i];
    }
    tofMutex.unlock();

    printMutex.lock();
    Serial.print("TOF: ");
    for (uint8_t i = 0; i < sensorCount; i++) {
      Serial.print(local[i]);
      if (i < sensorCount - 1) Serial.print(", ");
    }
    Serial.println();
    printMutex.unlock();

    ThisThread::sleep_for(SAMPLE_INTERVAL_MS * 1ms);
  }
}

void aiTask() {
  ei_impulse_result_t result = {0};

  // expects EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE floats
  static float inference_buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
  size_t buf_idx = 0;

  const uint32_t SAMPLE_INTERVAL_MS = 55;   // match TOF sampling
  const float GESTURE_THRESHOLD = 0.90f;    // stricter: 95% confidence
  uint32_t last_action_time = 0;
  const uint32_t COOLDOWN_MS = 2000;        // 2s cooldown between actions

  // extra gating
  const float MARGIN_THRESHOLD   = 0.25f;   // gesture must beat next class by 0.25
  const int   MOTION_THRESHOLD_MM = 50;    // min movement in mm to count as "motion"
  static int  consecutive_gesture = 0;

  static bool first_sample = true;
  static uint16_t prev_tof[4] = {0, 0, 0, 0};
  static bool window_has_motion = false;

  // Find index of "gesture" label
  int gesture_idx = -1;
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (strcmp(ei_classifier_inferencing_categories[i], "gesture") == 0) {
      gesture_idx = (int)i;
      break;
    }
  }
  if (gesture_idx < 0) {
    printMutex.lock();
    Serial.println("ERROR: 'gesture' label not found in model!");
    printMutex.unlock();
    return;
  }

  while (true) {
    // 1. Read latest TOF values under mutex
    uint16_t current_tof[4];
    tofMutex.lock();
    for (uint8_t i = 0; i < sensorCount; i++) {
      current_tof[i] = tofValues[i];
    }
    tofMutex.unlock();

    // --- motion detection vs previous sample ---
    if (first_sample) {
      for (int i = 0; i < 4; i++) prev_tof[i] = current_tof[i];
      first_sample = false;
    }

    int max_delta = 0;
    for (int i = 0; i < 4; i++) {
      int diff = (int)current_tof[i] - (int)prev_tof[i];
      if (diff < 0) diff = -diff;
      if (diff > max_delta) max_delta = diff;
    }
    for (int i = 0; i < 4; i++) prev_tof[i] = current_tof[i];

    if (max_delta > MOTION_THRESHOLD_MM) {
      window_has_motion = true;
    }

    // 2. Push one "frame" (4 sensors) into inference buffer
    inference_buffer[buf_idx++] = (float)current_tof[0];
    inference_buffer[buf_idx++] = (float)current_tof[1];
    inference_buffer[buf_idx++] = (float)current_tof[2];
    inference_buffer[buf_idx++] = (float)current_tof[3];

    // 3. When buffer full → 1 second of data collected
    if (buf_idx >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
      signal_t signal;

      int sf_res = numpy::signal_from_buffer(
        inference_buffer,
        EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
        &signal
      );

      if (sf_res != 0) {
        printMutex.lock();
        Serial.print("signal_from_buffer failed: ");
        Serial.println(sf_res);
        printMutex.unlock();
      } else {
        EI_IMPULSE_ERROR rc = run_classifier(&signal, &result, false);

        if (rc != EI_IMPULSE_OK) {
          printMutex.lock();
          Serial.print("run_classifier failed: ");
          Serial.println((int)rc);
          printMutex.unlock();
        } else {

          if (!window_has_motion) {
            // no motion in this window → ignore any "gesture" output
            printMutex.lock();
            Serial.println("ML: window had no motion, forcing no_gesture");
            printMutex.unlock();
            consecutive_gesture = 0;
          } else {
            // 4. Find top-2 labels
            float gesture_prob = result.classification[gesture_idx].value;

            int best_idx = 0;
            float best_prob = result.classification[0].value;
            int second_idx = -1;
            float second_prob = 0.0f;

            for (size_t i = 1; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
              float p = result.classification[i].value;
              if (p > best_prob) {
                second_prob = best_prob;
                second_idx = best_idx;
                best_prob = p;
                best_idx = (int)i;
              } else if (p > second_prob) {
                second_prob = p;
                second_idx = (int)i;
              }
            }

            float margin = best_prob - second_prob;

            // Debug all labels
            printMutex.lock();
            Serial.println("=== CLASSIFIER OUTPUT ===");
            for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
              Serial.print("  ");
              Serial.print(ei_classifier_inferencing_categories[i]);
              Serial.print(": ");
              Serial.println(result.classification[i].value, 3);
            }
            printMutex.unlock();

            // 5. Decide if gesture is real
            const float MARGIN_THRESHOLD = 0.15f; // difference between best & 2nd-best

            bool gesture_detected =
                (best_idx == gesture_idx) &&
                (best_prob >= GESTURE_THRESHOLD) &&
                (margin >= MARGIN_THRESHOLD);

            uint32_t now = millis();

            if (gesture_detected &&
                (now - last_action_time) >= COOLDOWN_MS) {

                printMutex.lock();
                Serial.println(">>> GESTURE DETECTED (single window), MOVING MOTOR");
                printMutex.unlock();

                motorMutex.lock();
                if (bright == 0) {
                  moveSteps(3, true, 100);
                  bright = 1;
                } else {
                  moveSteps(3, false, 100);
                  bright = 0;
                }
                motorMutex.unlock();

                last_action_time = now;   // reset cooldown
            }
          } // end window_has_motion check
        }   // end rc OK
      }     // end sf_res OK

      // reset for next 1-second window
      window_has_motion = false;
      buf_idx = 0;
    }

    ThisThread::sleep_for(SAMPLE_INTERVAL_MS * 1ms);
  }
}

void setup() {
  Serial.begin(115200);
  ThisThread::sleep_for(500ms);
  Wire.begin();
  Wire.setClock(400000);

  printMutex.lock();
  Serial.println("~--===STARTING SunSense===--~");
  printMutex.unlock();

  // --- Initialize motor driver pins ---
  enPin = 1; // start disabled
  slpPin = 1; // disable later
  stepPin = 0;
  dirPin = 0;
  ThisThread::sleep_for(100ms);

  luxInit();
  ThisThread::sleep_for(50ms);
  tofInit(50);
  ThisThread::sleep_for(50ms);

  printMutex.lock();
  Serial.println("SYSTEM: All sensors ready!");
  printMutex.unlock();

  // Start threads
  tofThread.start(tofTask);
  aiThread.start(aiTask);
  luxThread.start(luxTask);
}

void loop() {
}
