# HC-SR04 Ultrasonic Sensor for QNX
 
This repository contains sample code (in both C and Python) for interfacing with the HC-SR04 ultrasonic sensor on QNX. Originally written for the Trilobot platform, this code can also be changed for use on a breadboard or other hardware setups.
 
> **Important:**  
> Both implementations require root privileges to access the GPIO pins. The code uses QNX’s `rpi_gpio` API.
 
## Table of Contents
 
- [Overview](#overview)
- [Hardware Setup](#hardware-setup)
  - [Trilobot Setup](#trilobot-setup)
  - [Breadboard/Other Platforms](#breadboardother-platforms)
- [Schematic](#schematic)
- [Software Requirements](#software-requirements)
- [Build the code](#build-the-code)

 
## Overview
 
The HC-SR04 ultrasonic sensor measures distance by emitting an ultrasonic pulse and timing its echo. Both implementations perform the following tasks:
 
1. **Trigger the Sensor:**  
   A precise 10 µs pulse is sent on the trigger pin.
 
2. **Measure the Echo:**  
   The code waits for a rising edge followed by a falling edge on the echo pin.
 
3. **Calculate Distance:**  
   The elapsed time is converted to a distance (in centimeters) using the speed of sound.


## Hardware Setup
 
### Trilobot Setup
 
For the Trilobot platform, the HC-SR04 sensor is wired as follows:
 
- **VCC:** 5V (P1)
- **US_TRIG (Trigger):** Connected to GPIO 13 (P2)
- **US_ECHO (Echo):** Connected to GPIO 25 (P3)  
  *Note:* The sensor’s built-in resistor divider reduces the echo voltage (to ~2.5V).
- **GND:** Ground (P4)
 
In the Trilobot code, the internal pull resistor on the echo pin is typically disabled to allow the built-in divider to work properly. However, in some setups the sensor functions even when this is not disabled.
 
### Breadboard / Other Platforms
 
When using the sensor on a breadboard or other hardware:
 
- **Power:**  
  Ensure the sensor is supplied with a stable 5V and is properly grounded.
 
- **Connections:**  
  - **Trigger:** Connect to a GPIO output.  
  - **Echo:** Connect to a GPIO output. 
 
- **GPIO Pin Configuration:**  
  Modify the code to use the appropriate GPIO pins for your setup.
 
## Schematic
 
A schematic for the Trilobot wiring is available in the `schematic/` folder. This schematic shows the proper connections for the sensor on the Trilobot; you can use it as a reference when setting up your own circuit.
 
## Software Requirements
 
- **QNX Operating System 8.0**
- **rpi_gpio API:**  
  The GPIO client API (e.g., `rpi_gpio.h`) is provided in the `common/` folder.  

## Build the code

From the `hc-sr04_ultrasonic_sensor/c` directory, run: `make`
