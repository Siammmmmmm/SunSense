#include <Arduino.h>
#include "mbed.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <VL53L1X.h>

using namespace mbed;

// --- Pin Definitions ---
#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4 // DRV8825: LOW=enable, HIGH=disable
#define SLP_PIN 9 // LOW=disable, HIGH=enable

// --- tof ---
const uint8_t sensorCount = 4;

const uint8_t d1=0 , d2=0, d3=0, d4=0;

 //XSHUT pin of each sensor.
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
int lightThreshold = 6;    // lux value to trigger movement (adjust as needed)
int bright = 0;
// int stepDelay = 1000;       // microseconds between steps
// int stepsPerMove = 200;     // number of steps per movement

// --- objects ---
Mutex printMutex;   // guard Serial printing
Mutex motorMutex;   //guard motor operations
Thread luxThread;
Thread tofThread;

// Helper for safe printing
void safePrintln(const char *s) {
  printMutex.lock();
  Serial.println(s);
  printMutex.unlock();
}
void safePrintf(const char *fmt, ...) {
  printMutex.lock();
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printMutex.unlock();
}

void pulse(int microsec){
  stepPin = 1; wait_us(microsec);
  stepPin = 0; wait_us(microsec);
}

//motor moves
/*ex: 
2 full rotations = 2072 steps
full rotation = 1036 steps
half = 518 steps
quarter = 259 steps
*/
void moveSteps(int reps, bool dir, int freqHz){
  // Acqure motor lock
  motorMutex.lock();

  long steps = 1036 * reps;

  // Wake motor driver
  slpPin = 1; // awake
  dirPin = dir ? 1 : 0;
  enPin  = 0;                 // enable driver

  // Print
  {
    printMutex.lock();
    Serial.print("MOTOR : ");
    Serial.println(dir ? "Forward" : "Backward"); //true = clockwise : false = counterclockwise
    printMutex.unlock();
  }

  int microsec = (int)(1e6/(freqHz*2.0));
  for(long i=0;i<steps;i++) pulse(microsec);
  enPin  = 1;                 // disable outputs (no idle heat)
  slpPin = 0; //sleep

  motorMutex.unlock();
}

void tofInit(int freqHz){
  // --- Initialize tof sensors ---
  // Set all XSHUT pins as outputs and pull them LOW to disable all  tof sensors
  for (int i = 0; i < sensorCount; i++) {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }
  delay(10);

  for (int i = 0; i < sensorCount; i++) {
    // Stop driving this sensor's XSHUT low
    pinMode(xshutPins[i], INPUT);
    delay(10);

    // tof[i].setTimeout(500);
    if (!tof[i].init()) {
      printMutex.lock();
      Serial.print("Failed to detect and initialize sensor ");
      Serial.println(i);
      printMutex.unlock();
      continue;
    }

    tof[i].setDistanceMode(VL53L1X::Long);
    tof[i].setMeasurementTimingBudget(1000*freqHz);
    // tof[i].setROISize(14,14);
    // tof[i].setROICenter(199);

    // Each sensor must have its address changed to a unique value
    tof[i].setAddress(0x2A + i);
    tof[i].startContinuous(5+freqHz);
  }
}

void luxInit(){
  // --- Initialize light sensor ---
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
  const int stepsPerRotation = 1036;
  while (true) {
    // Read light
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

        // call motor
        long reps = 3;
        long steps = stepsPerRotation * reps;
        moveSteps(steps, true, 100);  // steps for 360 rotation * num of rotations

        bright = 1;
      } else if(event.light > lightThreshold && bright == 1){
        printMutex.lock();
        Serial.println("LIGHT : Bright enough -> rotating counterclockwise");
        printMutex.unlock();

        long reps = 3;
        long steps = stepsPerRotation * reps;
        moveSteps(reps, false, 100);

        bright = 0;
      }
    } else {
      printMutex.lock();
      Serial.println("LIGHT : No light data");
      printMutex.unlock();
    }

    // Sleep for a fixed period (non-blocking for other threads)
    ThisThread::sleep_for(1000ms);
  }
}

void tofTask(){
  while (true) {
    printMutex.lock();
    Serial.print(millis());
    printMutex.print(", ");
    printMutex.unlock();

    for (uint8_t i = 0; i < sensorCount; i++) {
      // read measurement
      int d = 0;
      bool ok = true;
      // Protect againt uninitialized sensors
      if (!tof[i].isAlive()) {
        ok = false;
      } else {
        d = tof[i].read();
        if (tof[i].timeoutOccured()) {
          // indicate timeout
          printMutex.lock();
          Serial.print("TOF");
          Serial.print(i);
          Serial.print(":TIMEOUT");
          printMutex.unlock();
          ok = false;
        }
      }
      // print result
      printMutex.lock();
      if (ok) {
        Serial.print("S");
        Serial.print(i+1);
        Serial.print(":");
        Serial.print(d);
        Serial.print("mm ");
      } else {
        Serial.print("S");
        Serial.print(i+1);
        Serial.print(":NA ");
      }
      printMutex.unlock();
    }
    printMutex.lock();
    Serial.println();
    printMutex.unlock();

    ThisThread::sleep_for(100ms)
  }
}

void ml(){
  
}

void setup() {
  Serial.begin(115200);
  // while(!Serial);
  delay(500);
  Wire.begin();
  Wire.setClock(400000);

  printMutex.lock();
  Serial.println("~--===STARTING SunSense===--~");
  printMutex.unlock();

  // --- Initialize motor driver pins ---
  enPin = 1; // start disabled
  slpPin = 1; //disable later
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
  luxThread.start(luxTask);
  tofThread.start(tofTask);
}

void loop() {
  ThisThread::sleep_for(1000ms);
}
