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

// --- Sensor Objects ---
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
//Adafruit_VL53L1X tof = Adafruit_VL53L1X();

// --- Parameters ---
int lightThreshold = 100    // lux value to trigger movement (adjust as needed)
int stepDelay = 1000;       // microseconds between steps
int stepsPerMove = 200;     // number of steps per movement

void setup() {
  Serial.begin(9600);
  Wire.begin();

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
  pinmode(ENABLE_PIN, OUTPUT);

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
      Serial.println("Too dark -> roating clockwise");
    }
  }
}
