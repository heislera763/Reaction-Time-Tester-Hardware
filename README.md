# Reaction Time Tester: Hardware
Recreating my Reaction Time Tester on hardware (Probably going to use an ESP32). Right now I am using the original RTT to create the skeleton of the firmware.

# Plans
The final implementation is going to use an external button, some LEDs to indicate the state, and an I2C output to a 1602 LCD. One complication is that I don't have a typical I2C to 1602 display module, so I will be implementing my own with an Arduino Pro Micro. 
