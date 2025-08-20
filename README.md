# Pi Hardware Component Samples

## Introduction

This repo contains a collection of Python and C samples to demonstrate using various hardware GPIO sensors with Raspberry Pi on QNX 8.0. Please feel free to contribute new samples or to tweak existing samples!

## Scope

All of the sample applications and APIs collected in this repository have been tested on Raspberry Pi 4B running the QNX 8.0 quick start target image.

Any modifications to the standard configuration required for any sample code to run will be noted in those sections.

## Hardware Components

| IC/Module              | Type                                     | Interface(s) | Used In Sample(s)                            | Languages |
|------------------------|------------------------------------------|--------------|----------------------------------------------|-----------|
| 1602 LCD               | Character Display                        | I2C          | [LCD 1602 Display](lcd-1602/)                | C, Python |
| 4‑digit 7‑segment     | Numeric Display                          | GPIO         | [4‑Digit 7‑Segment Display](4-digit-7-segment-display/) | C, Python |
| 7‑segment LED         | Single Digit Display                     | GPIO         | [7‑Segment Display](7-segment-display/)      | C, Python |
| BH1750                 | Light Sensor                             | I2C          | [Light Sensor BH1750](light-sensor-bh-1750/) | C |
| BNO055 (Adafruit)     | 9‑DoF IMU                               | I2C          | [Adafruit IMU BNO055](adafruit-imu-bno055/)  | C, Python |
| DRV8833                | Motor Controller                         | PWM, GPIO    | [Motor Controller](motor-controller/)        | C, Python |
| HC‑SR04                | Ultrasonic Distance Sensor               | GPIO         | [HC‑SR04 Ultrasonic](hc-sr04_ultrasonic_sensor/) | C, Python |
| HC‑SR501               | PIR Motion Sensor                        | GPIO         | [PIR Motion Sensor](pir-sensor/)             | C, Python |
| ICM‑20948 (SparkFun)  | 9‑DoF IMU                               | I2C          | [ICM‑20948 IMU](imu-icm20948/)               | C, Python |
| KY‑023                 | Analog Joystick                          | Analog       | [Analog Joystick](mcp-3008-joystick/), [Joystick Matrix Game](joystick-matrix-game/) | C |
| LED                    | Light Emitting Diode                     | GPIO         | [LED Blink](led-blink/), [LED Button](led-button/) | C, Python |
| LED (RGB)              | Multi-color Light                        | GPIO         | [LED RGB 4‑Wire](led-rgb-4-wire/)           | C, Python |
| MAX7219                | LED Matrix Driver                        | SPI          | [LED Matrix 8x8](led-max7219-matrix-8x8/), [Joystick Matrix Game](joystick-matrix-game/) | C |
| Matrix Keypad          | 4x4 Numeric Input                        | GPIO         | [Keypad Numeric](keypad-numeric/)            | C, Python |
| MCP3008                | Analog-to-Digital Converter              | SPI          | [Analog Joystick](mcp-3008-joystick/), [Joystick Matrix Game](joystick-matrix-game/) | C |
| PCF8574                | I/O Expander                             | I2C          | [LCD 1602 Display](lcd-1602/)                | C, Python |
| PCF8591                | ADC/DAC Converter                        | I2C          | [PCF8591 10k Pot](pcf8591/pcf8591-10kpot/), [PCF8591 Pot → LED](pcf8591/pcf8591-10kpot-led-control/), [PCF8591 GG Board](pcf8591/pcf8591-ggboard/) | C, Python |
| PCF8591 GG Board       | Multi-sensor Development Board           | I2C          | [PCF8591 GG Board](pcf8591/pcf8591-ggboard/) | C, Python |
| Pushbutton             | Digital Input Switch                     | GPIO         | [LED Button](led-button/), [Rotary Encoder](rotary-encoder/) | C, Python |
| Rotary Encoder         | Position/Rotation Sensor                 | GPIO         | [Rotary Encoder](rotary-encoder/)            | C, Python |
| SG90                   | Micro Servo Motor                        | PWM          | [Servo](servo/)                              | C, Python |
| SHT3X                  | Temperature & Humidity Sensor            | I2C          | [Temperature Humidity](temp-humidity-sht3x/) | C |
| SN3218A                | 18‑channel LED Driver                    | I2C          | [SN3218A RGB LED](sn3218a-rgb-led/)          | C, Python |
| WS281x                 | Addressable RGB LED                      | SPI          | [LED Strip Matrix](led-strip-matrix/)        | C |

## Content

### /common

This section of the repository contains useful system headers as well as client APIs that will be helpful to interact with hardware components.

[common](common)

## Development Host Setup Resources

Get a free, non-commercial QNX Software Development Platform 8.0 license at [https://www.qnx.com/getqnx](https://www.qnx.com/getqnx).

## Related Repositories

These sample projects are designed to be used alongside the following repositories:

### QNX 8.0 Quick Start Image (QSTI)
Provides a ready-to-use QNX image for Raspberry Pi.

https://gitlab.com/qnx/quick-start-images/raspberry-pi-qnx-8.0-quick-start-image

### Custom Target Image (CTI) Build for Raspberry Pi 4
A customizable build system for creating your own QNX 8.0 image.

https://gitlab.com/qnx/custom-target-image-builds/raspberry-pi-4-qnx-8.0

These repositories complement the sample code here and help you get up and running on real hardware.

## Contributor Guidelines

Feel free to submit changes or updates to this repo to resolve issues or improve features.
