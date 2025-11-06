#include "mbed.h"
#include <Arduino.h>

#define STEP_PIN     2
#define DIR_PIN      3
#define ENABLE_PIN   4

using namespace mbed;

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

void setup()
{
Serial.begin(115200);
  delay(500);

  enPin = 0; // LOW = Enable driver
  stepPin = 0;
  dirPin = 0;
  
  analogReadResolution(12);  
}
void loop()
{
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
}