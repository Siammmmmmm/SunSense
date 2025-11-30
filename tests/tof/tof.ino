#include <Wire.h>
#include <Adafruit_VL53L1X.h>

// Define XSHUT pins for each sensor
#define XSHUT1 5
#define XSHUT2 6
#define XSHUT3 7
#define XSHUT4 8

// Create 4 sensor objects with different I2C addresses
Adafruit_VL53L1X tof1 = Adafruit_VL53L1X();
Adafruit_VL53L1X tof2 = Adafruit_VL53L1X();
Adafruit_VL53L1X tof3 = Adafruit_VL53L1X();
Adafruit_VL53L1X tof4 = Adafruit_VL53L1X();
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // wait for Serial to connect

  // Initialize I2C
  Wire.begin();

  // Set all XSHUT pins as outputs and pull them LOW to disable all sensors
  pinMode(XSHUT1, OUTPUT);
  digitalWrite(XSHUT1, LOW);
  pinMode(XSHUT2, OUTPUT);
  digitalWrite(XSHUT2, LOW);
  pinMode(XSHUT3, OUTPUT);
  digitalWrite(XSHUT3, LOW);
  pinMode(XSHUT4, OUTPUT);
  digitalWrite(XSHUT4, LOW);
  delay(10);

  // --- Initialize Sensor 1 ---
  digitalWrite(XSHUT1, HIGH);  // Enable sensor 1
  delay(10);
  if (!tof1.begin(0x30, &Wire)) {  // Assign address 0x30
    Serial.println("TOF Sensor 1 failed!");
    while (1)
      ;
  }
  tof1.VL53L1X_SetDistanceMode(3);  //change ranging distance to long
  tof1.setTimingBudget(100000); //100ms == 10Hz
  tof1.VL53L1X_SetInterMeasurementInMs(110);
  Serial.println("TOF Sensor 1 initialized at 0x30");

  // --- Initialize Sensor 2 ---
  digitalWrite(XSHUT2, HIGH);  // Enable sensor 2
  delay(10);
  if (!tof2.begin(0x31, &Wire)) {  // Assign address 0x31
    Serial.println("TOF Sensor 2 failed!");
    while (1)
      ;
  }
  tof2.VL53L1X_SetDistanceMode(3);  //change ranging distance to long
  tof2.setTimingBudget(100000); 
  tof2.VL53L1X_SetInterMeasurementInMs(110);
  Serial.println("TOF Sensor 2 initialized at 0x31");

  // --- Initialize Sensor 3 ---
  digitalWrite(XSHUT3, HIGH);  // Enable sensor 3
  delay(10);
  if (!tof3.begin(0x32, &Wire)) {  // Assign address 0x32
    Serial.println("TOF Sensor 3 failed!");
    while (1)
      ;
  }
  tof3.VL53L1X_SetDistanceMode(3);  //change ranging distance to long
  tof3.setTimingBudget(100000); 
  tof3.VL53L1X_SetInterMeasurementInMs(110);
  Serial.println("TOF Sensor 3 initialized at 0x32");

  // --- Initialize Sensor 4 ---
  digitalWrite(XSHUT4, HIGH);  // Enable sensor 4
  delay(10);
  if (!tof4.begin(0x33, &Wire)) {  // Assign address 0x33
    Serial.println("TOF Sensor 4 failed!");
    while (1)
      ;
  }
  tof4.VL53L1X_SetDistanceMode(3);  //change ranging distance to long
  tof4.setTimingBudget(100000); 
  tof4.VL53L1X_SetInterMeasurementInMs(110);
  Serial.println("TOF Sensor 4 initialized at 0x33");

  // Start ranging on all sensors
  tof1.startRanging();
  tof2.startRanging();
  tof3.startRanging();
  tof4.startRanging();

  Serial.println("All sensors ready!");
}

void loop() {
  int d1 = tof1.distance();
  delay(120);
  int d2 = tof2.distance();
  delay(120);
  int d3 = tof3.distance();
  delay(120);
  int d4 = tof4.distance();
  delay(120);

  Serial.print(millis()); Serial.print(","); Serial.print(d1); Serial.print((",")); Serial.print(d2); Serial.print((",")); Serial.print(d3); Serial.print((",")); Serial.println((d4));
}