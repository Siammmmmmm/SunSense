# SunSense
Automated Smart Blinds System

A real-time embedded system that automatically opens or closes window blinds based on ambient light level and user gestures. The system uses light sensors to detect brightness conditions and Time-of-Flight sensors with TinyML to recognize simple hand gestures for manual override.

# Setup and Deployment

To build this project, you'll need an Arduino Nano 33 BLE Sense Rev2, a Nema 17 stepper motor with DRV8825 driver, one TSL2561 light sensor, four VL53L1X Time of Flight sensors, and a 12V 3A power supply. Install the required Arduino libraries (Adafruit TSL2561, Adafruit VL53L1X, and Adafruit Unified Sensor) through the Arduino IDE Library Manager. Wire the TSL2561 to the Arduino's I2C pins (SDA/SCL) and 3.3V power. Connect all four VL53L1X sensors to the same I2C bus with their XSHUT pins connected to digital pins D2-D5 to assign unique addresses. Wire the DRV8825 motor driver to the 12V power supply and connect its control pins (STEP, DIR, ENABLE) to Arduino pins D8, D9, and D10, then connect the stepper motor to the driver's output pins. Clone the code and upload Main/SunSense.ino to your Arduino, and attach the motor to your blind rode using the 3D printed coupling.

# How to Use and Interact

Once powered on, the system automatically reads the light sensor and controls the blinds accordingly - closing them when it's dark and opening them when it's bright. The light threshold can be adjusted in the code if needed. For manual control, wave your hand within a few feet of the four ToF sensors to trigger gesture detection and overried the automatic mode. The system runs continuously and will keep monitoring light levels to adjust the blinds in real time. Individual component tests are available in the Tests folder if you need to verify each sensor or the motor is working correctly before running the full system.
