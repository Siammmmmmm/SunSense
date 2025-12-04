#include <Arduino.h>
#include "mbed.h"
#include <rtos.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

using namespace mbed;
using namespace rtos;
using namespace std::chrono_literals;

// --- Pin Definitions ---
#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4 // DRV8825: LOW=enable, HIGH=disable
#define SLP_PIN 9    // LOW=disable, HIGH=enable

// --- motor out ---
DigitalOut stepPin(digitalPinToPinName(STEP_PIN));
DigitalOut dirPin (digitalPinToPinName(DIR_PIN));
DigitalOut enPin  (digitalPinToPinName(ENABLE_PIN));
DigitalOut slpPin (digitalPinToPinName(SLP_PIN));

// --- Sensor Objects ---
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

// --- Parameters ---
const int lightThreshold = 6;    // lux value to trigger movement (adjust as needed)
int bright = 1;

// --- RTOS objects ---
Mutex printMutex; // guard Serial printing
Mutex motorMutex; // guard motor operations

Thread luxThread;

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
  motorMutex.unlock();
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

  printMutex.lock();
  Serial.println("SYSTEM: All sensors ready!");
  printMutex.unlock();

  // Start threads
  luxThread.start(luxTask);
}

void loop() {
}
