#include <Wire.h>
#include <VL53L1X.h>

// The number of tof in your system.
const uint8_t sensorCount = 4;

const uint8_t d1=0 , d2=0, d3=0, d4=0;

// The Arduino pin connected to the XSHUT pin of each sensor.
const uint8_t xshutPins[sensorCount] = { 5, 6, 7, 8 };

VL53L1X tof[sensorCount];

void setup() {
  while (!Serial) 
  Serial.begin(115200);
  delay(10);  // wait for Serial to connect

  // Initialize I2C
  Wire.begin();
  Wire.setClock(400000);

  // Set all XSHUT pins as outputs and pull them LOW to disable all sensors
  for (int i = 0; i < sensorCount; i++)
  {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }

  delay(10);

  for (int i = 0; i < sensorCount; i++)
  {
    // Stop driving this sensor's XSHUT low
    pinMode(xshutPins[i], INPUT);
    delay(10);

    // tof[i].setTimeout(500);
    if (!tof[i].init())
    {
      Serial.print("Failed to detect and initialize sensor ");
      Serial.println(i);
      while (1);
    }

    tof[i].setDistanceMode(VL53L1X::Long);
    tof[i].setMeasurementTimingBudget(140000);
    tof[i].setROISize(14,14);
    tof[i].setROICenter(199);

    // Each sensor must have its address changed to a unique value
    tof[i].setAddress(0x2A + i);
    tof[i].startContinuous(145);
  }

  Serial.println("All sensors ready!");
}

void loop() {
  Serial.print(millis());

  for (uint8_t i = 0; i < sensorCount; i++)
  {
    Serial.print(",");
    Serial.print(tof[i].read());
    if (tof[i].timeoutOccurred()) {
       Serial.print(" TIMEOUT"); 
    }
  }
  Serial.println();

  // Serial.print(millis());Serial.print(d1); Serial.print((",")); Serial.print(d2); Serial.print((",")); Serial.print(d3); Serial.print((",")); Serial.println((d4));
}