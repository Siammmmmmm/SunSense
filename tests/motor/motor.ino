#include <Arduino.h>
#include "mbed.h"

#define STEP_PIN 2
#define DIR_PIN  3
#define EN_PIN   4   // DRV8825: LOW=enable, HIGH=disable

using namespace mbed;
DigitalOut stepPin(digitalPinToPinName(STEP_PIN));
DigitalOut dirPin (digitalPinToPinName(DIR_PIN));
DigitalOut enPin  (digitalPinToPinName(EN_PIN));

void pulseOnce(int microsec){
  stepPin = 1; wait_us(microsec);
  stepPin = 0; wait_us(microsec);
}

// simple move with enable only during motion
void moveSteps(long steps, bool dir, int freqHz){
  Serial.println(dir ? "Forward" : "Backward");
  dirPin = dir ? 1 : 0;
  enPin  = 0;                 // enable driver
  int microsec = (int)(1e6/(freqHz*2.0));
  for(long i=0;i<steps;i++) pulseOnce(microsec);
  enPin  = 1;                 // disable outputs (no idle heat)
}

void setup(){
  enPin = 1;                  // start disabled
  stepPin = 0;
  dirPin = 0;
}

void loop(){
  // Example: ~1/4 turn (≈259 steps on 5.18:1)
  moveSteps(1036, true, 80);   // start slow (50–100 Hz)
  delay(1500);
  moveSteps(1036, false, 80);
  delay(3000);                // plenty of idle time with coils off
}
