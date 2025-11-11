#include <Arduino.h>
#include "mbed.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
// #include <Adafruit_VL53L1X.h>

// --- Pin Definitions ---
#define STEP_PIN     2
#define DIR_PIN      3
#define ENABLE_PIN   4

using namespace mbed;

//--motor out--
DigitalOut stepPin(digitalPinToPinName(STEP_PIN));
DigitalOut dirPin (digitalPinToPinName(DIR_PIN));
DigitalOut enPin  (digitalPinToPinName(ENABLE_PIN));

// --- Sensor Objects ---
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
//Adafruit_VL53L1X tof = Adafruit_VL53L1X();

// --- Parameters ---
int lightThreshold = 100;    // lux value to trigger movement (adjust as needed)
int stepDelay = 1000;       // microseconds between steps
int stepsPerMove = 200;     // number of steps per movement

void pulse(int microsec){
  stepPin = 1; wait_us(microsec);
  stepPin = 0; wait_us(microsec);
}

//motor moves
/*ex: 
full rotation = 1036 steps
half = 518 steps
quarter = 259 steps
*/
void moveSteps(long steps, bool dir, int freqHz){ //freqHZ: 50-100!
  Serial.println(dir ? "Forward" : "Backward"); 
  dirPin = dir ? 1 : 0;//true = clockwise : false = counterclockwise
  enPin  = 0;                 // enable driver
  int microsec = (int)(1e6/(freqHz*2.0));
  for(long i=0;i<steps;i++) pulse(microsec);
  enPin  = 1;                 // disable outputs (no idle heat)
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  delay(500);

  // --- Initialize motor ---
  enPin = 1; // default its disabled to reduce heat
  stepPin = 0;
  dirPin = 0;

  // --- Initialize light sensor ---
  if (!tsl.begin()) {
    Serial.println("No TSL2561 found!");
    while (1);
  } else {
    Serial.println("TSL2561 initialized!");
    tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  }

  // --- Initialize motor driver pins ---
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  digitalWrite(ENABLE_PIN, LOW);
}

void loop() {
  sensors_event_t event;
  tsl.getEvent(&event);

  if (event.light) {
    Serial.print("Light: ");
    Serial.print(event.light);
    Serial.print(" lux");

    if (event.light < lightThreshold) {
      Serial.println("Too dark -> rotating clockwise");
      moveSteps(1036, true, 80);  // one rotation clockwise
    } else {
      Serial.println("Bright enough -> rotating counterclockwise");
      moveSteps(1036, false, 80);
    }
  } else {
    Serial.println("No light data");
  }

  delay(1000);
}
