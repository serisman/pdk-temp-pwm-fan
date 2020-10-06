# pdk-temp-pwm-fan
A temperature controlled PWM fan controller using an inexpensive (3 cent) 8-bit Padauk microcontroller.

### Links
- [Firmware](firmware/)
- [PCB](pcb-u06/)

This fan controller drives a common 4-pin PC fan, by sending a 25 kHz PWM signal with a varying duty cycle.
The duty cycle is adjusted up or down based on a high and low temperature set point.
- If the temperature is below the low set point, the PWM duty is decreased by 1 every 1/3 second until it the fan is off (or as slow as it can go).
- If the temperature is above the high set point, the PWM duty cycle is increased by 1 times the number increments above the high set point every 1/3 second until the fan is fully on.

The temperature is measured using a 10K NTC thermistor between VCC and a 1K resistor to GND forming a voltage divider.
- The output of this voltage divider is fed into the Padauk microcontroller's Comparator + input.
- The Comparator's - input is connected to an internally generated voltage source that can be set to 16 different voltage levels between GND and 1/2 VCC.
- So, by changing the internal voltage source, the Comparator can be checked to see if the NTC's output is greater or less than a set point.

This temperature measurement isn't particularly precise, but should suffice for crude temperature control requirements,
like cooling down a heatsink in a power supply or UPS, with minimal power loss and noise when cooling isn't required.

### Schematic
> ![Schematic](https://github.com/serisman/pdk-temp-pwm-fan/blob/master/pcb-u06/output/Schematic.png?raw=true)

### Compatibility
This project is currently intended to be run on the PMS150C Padauk microcontrollers,
but it should be able to be run on pretty much any other Padauk microcontrollers that have a Comparator (most)
and that are supported by SDCC and the Easy PDK Programmer.

### Images
![Front](https://github.com/serisman/pdk-temp-pwm-fan/blob/master/img/Front.jpg?raw=true)
![Back](https://github.com/serisman/pdk-temp-pwm-fan/blob/master/img/Back.jpg?raw=true)
![Finished!](https://github.com/serisman/pdk-temp-pwm-fan/blob/master/img/Finished.jpg?raw=true)

### Copyright and License:
- Copyright (C) 2020 - serisman (github@serisman.com)
- License: [GPL v3 (or later)](LICENSE)
