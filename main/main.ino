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
Adafruit_TSL2561_Unified light = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
Adafruit_VL53L1X tof = Adafruit_VL53L1X();



void setup() {
  Serial.begin(115200);
  delay(500);

  enPin = 0; // LOW = Enable driver
  stepPin = 0;
  dirPin = 0;
  
  analogReadResolution(12); // 12-bit ADC on Nano 33 BLE

  
    // // --- Initialize TSL2561 ---
    // if(!light.begin()) {
    //     Serial.println("TSL2561 not found!");
    // } else {
    //     Serial.println("TSL2561 found!");
    //     light.enableAutoRange(true);
    //     light.setIntegrationTime(TSL_2561_INTEGRATIONTIME_13MS);
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


// // --- Read TSL2561 ---
//     sensors_event_t event;
//     light.getEvent(&event);
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
