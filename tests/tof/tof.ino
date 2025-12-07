#include <Wire.h>
#include <VL53L1X.h>

// button pin
// const int BTN = 10;

// The number of tof in your system.
const uint8_t sensorCount = 4;

const uint8_t d1=0 , d2=0, d3=0, d4=0;

// The Arduino pin connected to the XSHUT pin of each sensor.
const uint8_t xshutPins[sensorCount] = { 5, 6, 7, 8 };

VL53L1X tof[sensorCount];

unsigned long last = 0;
bool running = false;
int count = 0;

void setup() {
  while (!Serial) 
  Serial.begin(115200);
  delay(10);  // wait for Serial to connect

  // Initialize I2C
  Wire.begin();
  Wire.setClock(400000);

  // Initialize button
  // pinMode(BTN, INPUT_PULLUP);

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
    tof[i].setMeasurementTimingBudget(20000);
    // tof[i].setROISize(14,14);
    // tof[i].setROICenter(199);

    // Each sensor must have its address changed to a unique value
    tof[i].setAddress(0x2A + i);
    tof[i].startContinuous(50);
  }

  Serial.println("All sensors ready!");
}

void loop() {
  if (Serial.available()) {
      char c = Serial.read();
      if (c == 's') {
        Serial.println();
        Serial.println("=============================START");  // marker
        delay(750);
      }
  }
  // unsigned long now = millis();

  // if(now - last  >= 1450){
  //   running = !running;
  //   last = now;
  // }


  // if (running){
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
  // } else {
  //   Serial.println();
  // }



  // Serial.print(millis());Serial.print(d1); Serial.print((",")); Serial.print(d2); Serial.print((",")); Serial.print(d3); Serial.print((",")); Serial.println((d4));
}