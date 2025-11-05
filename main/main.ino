#include <Arduino.h>
#include "mbed.h"
// #include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_TSL2561_U.h>
// #include <Adafruit_VL53L1X.h>

// --- Pin Definitions ---
#define STEP_PIN     2
#define DIR_PIN      3
#define ENABLE_PIN   4

using namespace mbed;

//// --- Sensor Objects ---
// Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
// Adafruit_VL53L1X tof = Adafruit_VL53L1X();

// Use MBed DigitalOut for fast & stable pin toggling
DigitalOut stepPin(digitalPinToPinName(STEP_PIN));
DigitalOut dirPin (digitalPinToPinName(DIR_PIN));
DigitalOut enPin  (digitalPinToPinName(ENABLE_PIN));

void stepMotor(int steps, bool direction, int pulseDelay_us) {
  dirPin = direction ? 1 : 0;   // Set direction
  for (int i = 0; i < steps; i++) {
    stepPin = 1;
    wait_us(pulseDelay_us);
    stepPin = 0;
    wait_us(pulseDelay_us);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  enPin = 0; // LOW = Enable driver
  stepPin = 0;
  dirPin = 0;
  
  analogReadResolution(12); // 12-bit ADC on Nano 33 BLE

  
    // // --- Initialize TSL2561 ---
    // if(!tsl.begin()) {
    //     Serial.println("TSL2561 not found!");
    // } else {
    //     Serial.println("TSL2561 found!");
    //     tsl.enableAutoRange(true);
    //     tsl.setIntegrationTime(TSL_2561_INTEGRATIONTIME_13MS);
    // }
    
    // // --- Initialize VL53L1X ---
    // if(!tof.begin()) {
    //     Serial.println("VL53L1X not found!");
    // } else {
    //     Serial.println("VL53L1X found!");
    //     tof.startContinuous(50); // continuous mode, 50ms interval 
    // }

  Serial.println("=== Motor Test Starting ===");
}

void loop() {
    uint16_t raw = analogRead(A0);
    float v = (raw / 4095.0f) * 3.3f;  // ADC range is 0..3.3V
    Serial.print("Vref = "); Serial.print(v, 3); Serial.println(" V");
    delay(300);

    Serial.println("Forward...");
    stepMotor(1000, true, 2000);   // 1000 steps forward, slow for torque
    delay(1500);

    Serial.println("Backward...");
    stepMotor(1000, false, 2000);  // 1000 steps backward
    delay(1500);

// // --- Read TSL2561 ---
//     sensors_event_t event;
//     tsl.getEvent(&event);
//     if (event.light) {
//         Serial.print("Light: ");
//         Serial.print(event.light);
//         Serial.println(" lux");
//     } else {
//         Serial.println("No light data");
//     }

//     // --- Read Vl53L1X ---
//     Serial.print("Distance: ");
//     Serial.print(tof.read());
//     Serial.println(" mm");
  
}
