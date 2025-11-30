#include <Arduino.h>
#include "mbed.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_VL53L1X.h>

// --- Pin Definitions ---
#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4 // DRV8825: LOW=enable, HIGH=disable
#define SLP_PIN 9 // LOW=disable, HIGH=enable
#define XSHUT1 5
#define XSHUT2 6
#define XSHUT3 7
#define XSHUT4 8

using namespace mbed;

//--motor out--
DigitalOut stepPin(digitalPinToPinName(STEP_PIN));
DigitalOut dirPin (digitalPinToPinName(DIR_PIN));
DigitalOut enPin  (digitalPinToPinName(ENABLE_PIN));
DigitalOut slpPin (digitalPinToPinName(SLP_PIN));


// --- Sensor Objects ---
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

Adafruit_VL53L1X tof1 = Adafruit_VL53L1X();
Adafruit_VL53L1X tof2 = Adafruit_VL53L1X();
Adafruit_VL53L1X tof3 = Adafruit_VL53L1X();
Adafruit_VL53L1X tof4 = Adafruit_VL53L1X();

// --- Parameters ---
int lightThreshold = 6;    // lux value to trigger movement (adjust as needed)
int bright = 1;
// int stepDelay = 1000;       // microseconds between steps
// int stepsPerMove = 200;     // number of steps per movement

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
void moveSteps(long steps, bool dir, int freqHz){
  slpPin = 1; // awake
  Serial.print("MOTOR : ");
  Serial.println(dir ? "Forward" : "Backward"); //true = clockwise : false = counterclockwise
  dirPin = dir ? 1 : 0;
  enPin  = 0;                 // enable driver
  int microsec = (int)(1e6/(freqHz*2.0));
  for(long i=0;i<steps;i++) pulse(microsec);
  enPin  = 1;                 // disable outputs (no idle heat)
  slpPin = 0; //sleep
}

void setup() {
  Serial.begin(9600);
  // while (!Serial){};
  delay(500);

  Wire.begin();
  Serial.println("~--===STARTING SunSense===--~");

  // --- Initialize motor driver pins ---
  enPin = 1;                  // start disabled
  slpPin = 1; //disable later
  stepPin = 0;
  dirPin = 0;

  delay(100);
  // --- Initialize light sensor ---
  if (!tsl.begin()) {
    Serial.println("SYSTEM: No TSL2561 found!");
    while (1);
  } else {
    Serial.println("SYSTEM: TSL2561 initialized!");
    tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  }
  delay(10);

  // Set all XSHUT pins as outputs and pull them LOW to disable all sensors
  pinMode(XSHUT1, OUTPUT);
  pinMode(XSHUT2, OUTPUT);
  pinMode(XSHUT3, OUTPUT);
  pinMode(XSHUT4, OUTPUT);

  digitalWrite(XSHUT1, LOW);
  digitalWrite(XSHUT2, LOW);
  digitalWrite(XSHUT3, LOW);
  digitalWrite(XSHUT4, LOW);
  delay(10);

  // --- Initialize Sensor 1 ---
  digitalWrite(XSHUT1, HIGH);     // Enable sensor 1
  delay(10);
  if(!tof1.begin(0x30, &Wire)) {   // Assign address 0x30
      Serial.println("SYSTEM: TOF Sensor 1 failed!");
      while (1);
  }
  Serial.println("SYSTEM: TOF Sensor 1 initialized at 0x30"); 

  // --- Initialize Sensor 2 ---
  digitalWrite(XSHUT2, HIGH);     // Enable sensor 2
  delay(10);
  if(!tof2.begin(0x31, &Wire)) {   // Assign address 0x31
      Serial.println("SYSTEM: TOF Sensor 2 failed!");
      while (1);
  }
  Serial.println("SYSTEM: TOF Sensor 2 initialized at 0x31"); 

  // --- Initialize Sensor 3 ---
  digitalWrite(XSHUT3, HIGH);     // Enable sensor 3
  delay(10);
  if(!tof3.begin(0x32, &Wire)) {   // Assign address 0x32
      Serial.println("SYSTEM: TOF Sensor 3 failed!");
      while (1);
  }
  Serial.println("SYSTEM: TOF Sensor 3 initialized at 0x32"); 

  // --- Initialize Sensor 4 ---
  digitalWrite(XSHUT4, HIGH);     // Enable sensor 4
  delay(10);
  if(!tof4.begin(0x33, &Wire)) {   // Assign address 0x33
      Serial.println("SYSTEM: TOF Sensor 4 failed!");
      while (1);
  }
  Serial.println("SYSTEM: TOF Sensor 4 initialized at 0x33"); 

  // Start ranging on all sensors
  tof1.startRanging();
  tof2.startRanging();
  tof3.startRanging();
  tof4.startRanging();

  Serial.println("SYSTEM: All sensors ready!");
}

void loop() {
  sensors_event_t event;
  tsl.getEvent(&event);

  if (event.light) {
    Serial.print("LIGHT : ");
    Serial.print(event.light);
    Serial.println(" lux!");


    if (event.light < lightThreshold && bright == 0) {
      Serial.println("LIGHT : Too dark -> rotating clockwise");
      moveSteps(1036*3, true, 100);  // steps for 360 rotation * num of rotations
      bright = 1;
    } else if(event.light > lightThreshold && bright == 1){
      Serial.println("LIGHT : Bright enough -> rotating counterclockwise");
      moveSteps(1036*3, false, 100);
      bright = 0;
    }
  } else {
    Serial.println("LIGHT : No light data");
  }

  // Read tof Sensor 1
  if (tof1.dataReady()) {
      int distance1 = tof1.distance();  // get distance in mm
      Serial.print("TOF 1 : ");
      Serial.print(distance1);
      Serial.println(" mm  ");
      tof1.clearInterrupt();  // clear the interrupt so new data can come in
  }

  // Read tof Sensor 2
  if (tof2.dataReady()) {
      int distance2 = tof2.distance();  // get distance in mm
      Serial.print("TOF 2 : ");
      Serial.print(distance2);
      Serial.println(" mm  ");
      tof2.clearInterrupt();  // clear the interrupt so new data can come in
  }

  // Read tof Sensor 3
  if (tof3.dataReady()) {
      int distance3 = tof3.distance();  // get distance in mm
      Serial.print("TOF 3 : ");
      Serial.print(distance3);
      Serial.println(" mm  ");
      tof3.clearInterrupt();  // clear the interrupt so new data can come in
  }

  // Read tof Sensor 4
  if (tof4.dataReady()) {
      int distance4 = tof4.distance();  // get distance in mm
      Serial.print("TOF 4 : ");
      Serial.print(distance4);
      Serial.println(" mm  ");
      tof4.clearInterrupt();  // clear the interrupt so new data can come in
  }

  delay(1000);
}
