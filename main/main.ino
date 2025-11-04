#include "mbed.h"

// --- Pin Definitions ---
#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4

void setup() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    
    // Enable driver (LOW = ON for DRV8825)
    digitalWrite(ENABLE_PIN, LOW);

    // Set intitial direction
    digitalWrite(DIR_PIN, HIGH);
}

void loop() {
    // Rotate one direction
    digitalWrite(DIR_PIN, HIGH);
    for (int i = 0; i < 200; i++) {     // 200 steps = 1 revolution
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(1000);        // pulse width
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(1000);
    }

    delay(1000);    // pause 1 second

    // Rotate the other direction
    digitalWrite(DIR_PIN, LOW);
    for (int i = 0; i < 200; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(1000);
        digitalWrtie(STEP_PIN, LOW);
        delayMicroseconds(1000);
    }

    delay(1000);
}