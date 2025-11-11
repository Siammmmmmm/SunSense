#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

Adafruit_TSL2561_Unified light = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

void setup() {
    Serial.begin(9600);
    delay(1000);

    Serial.println("Starting TSL2561 test...");

    // --- Initialize TSL2561 ---
    if(!light.begin()) {
        Serial.println("TSL2561 not found!");
        while (1); // stop here if not found;
    } else {
        Serial.println("TSL2561 found!");
        light.enableAutoRange(true);
        light.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
     }
}

void loop() {
    // --- Read TSL2561 ---
    sensors_event_t event;
    light.getEvent(&event);
    if (event.light) {
        Serial.print("Light: ");
        Serial.print(event.light);
        Serial.println(" lux");
    } else {
        serial.println("No light data");
    } 

    delay(1000); // read every second
}    
