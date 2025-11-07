#include <Arduino.h>

#define STEP_PIN 2
#define DIR_PIN  3
// EN is HARD-WIRED to GND for this test
// RESET+SLEEP HARD-WIRED to 3.3V
// MS1/2/3 HARD-WIRED to GND

void step_once(unsigned long ms) {
  digitalWrite(STEP_PIN, HIGH); delay(ms);
  digitalWrite(STEP_PIN, LOW);  delay(ms);
}

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, HIGH);  // forward
}

void loop() {
  // 40 very-slow steps forward (~20 Hz -> 25 ms high + 25 ms low)
  for (int i=0; i<40; i++) step_once(25);
  delay(800);

  // 40 very-slow steps backward
  digitalWrite(DIR_PIN, LOW);
  for (int i=0; i<40; i++) step_once(25);
  delay(1200);
}
