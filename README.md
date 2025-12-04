# SunSense
Automated Smart Blinds System

A real-time embedded system that automatically opens or closes window blinds based on ambient light level. The system uses light sensors to detect brightness conditions.

# Setup and Deployment

To build this project, you'll need an Arduino Nano 33 BLE Sense Rev2, a Nema 17 stepper motor with DRV8825 driver, one TSL2561 light sensor, and a 12V 3A power supply. Install the required Arduino libraries (Adafruit TSL2561, and Adafruit Unified Sensor) through the Arduino IDE Library Manager. Wire the TSL2561 to the Arduino's I2C pins (SDA/SCL) and 3.3V power. Wire the DRV8825 motor driver to the 12V power supply and connect its control pins: STEP to pin 2, DIR to pin 3, ENABLE to pin 4, and SLP (sleep) to pin 9. Connect the stepper motor to the driver's output pins. Clone the code and upload Main/SunSense.ino to your Arduino. Attach the motor to your blind rod using the 3D printed coupling that allows it to rotate the rod.

# How to Use and Interact

Once powered on, the light sensing thread checks the ambient light level every 10 seconds and rotates the motor 3 full rotations to open or close the blinds when the light crosses the threshold of 6 lux. All activity is logged to the Serial Monitor at 115200 baud if you want to debug or monitor the system's behavior.
