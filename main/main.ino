#include "mbed.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_VL53L1X.h>

// --- Pin Definitions ---
#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4

// --- Sensor Objects ---
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
Adafruit_VL53L1X tof = Adafruit_VL53L1X();

void setup() {
    Serial.begin(9600);

    // Initialize I2C
    Wire.begin();

    // Motor pins
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    
    // Enable driver (LOW = ON for DRV8825)
    digitalWrite(ENABLE_PIN, LOW);

    // Set initial direction
    digitalWrite(DIR_PIN, HIGH);



    // --- Initialize TSL2561 ---
    if(!tsl.begin()) {
        Serial.println("TSL2561 not found!");
    } else {
        Serial.println("TSL2561 found!");
        tsl.enableAutoRange(true);
        tsl.setIntegrationTime(TSL_2561_INTEGRATIONTIME_13MS);
    }
    
    // --- Initialize VL53L1X ---
    if(!tof.begin()) {
        Serial.println("VL53L1X not found!");
    } else {
        Serial.println("VL53L1X found!");
        tof.startContinuous(50); // continuous mode, 50ms interval 
    }
}

void loop() {
    // --- Motor Test ---
    // Rotate one direction
    digitalWrite(DIR_PIN, HIGH);
    for (int i = 0; i < 200; i++) {     // 200 steps = 1 revolution
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(1000);        // pulse width
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(1000);
    }

    delay(1000);    // pause 1 second

    // Rotate the other direction
    digitalWrite(DIR_PIN, LOW);
    for (int i = 0; i < 200; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(1000);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(1000);
    }

    delay(1000);

    // --- Read TSL2561 ---
    sensors_event_t event;
    tsl.getEvent(&event);
    if (event.light) {
        Serial.print("Light: ");
        Serial.print(event.light);
        Serial.println(" lux");
    } else {
        Serial.println("No light data");
    }

    // --- Read Vl53L1X ---
    Serial.print("Distance: ");
    Serial.print(tof.read());
    Serial.println(" mm");

    delay(500); // half-second pause between readings
}