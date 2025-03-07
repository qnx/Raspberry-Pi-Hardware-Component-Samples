# WS281x LED Strips and Matrices

## Overview

This section will contain several sample applications for driving LED strips and matrices using
the RaspBerry PI's SPI0 device.

The sample applications use two or more of the following libraries:

- librpi_spi (all sample applications)
- librpi_ws281x (all_sample applications)
- libmini_fastled (all but one sample applications)

### librpi_spi

This library provides the underlying logic to communicate with the SPI driver.  Sample applications only have to link this library
and they are not accessing its APIs directly.

### librpi_ws281x

This library provides a somewhat generic API to write colours to a collection of WS 281x LEDs.  It is suitable for simpler control of
one or more LED strips or a LED matrix but it does not contain specialized logic for managing matrices in a transparent fashion.

This library is a smaller port of the Linux library of the same name (https://github.com/jgarff/rpi_ws281x) but the port focuses only 
on the SPI driver of the original library but the implementation of the SPI logic is based on librpi_spi.  The interface has been mostly
retained but some minor changes were made because the port only covers SPI functionality.

The [rpi-ws281x-rainbow](rpi-ws281x-rainbow) sample application demonstrates how to use this API.

### libmini_fastled
 
The mini_fastled library is based on the well known FastLED library (https://fastled.io/) but is not a complete port, at least not yet.
The QNX port has some significant differences from the originals
- The QNX port is based on a C API instead of C++, but the API is meant to be similar in some ways to the INO script APIs for FastLED, so that
  translation from INO script to C code is not too onerous.
- The library is layered on librpi_ws281x, so certain types are matched under the hood to that library's datatypes.
- Some of the lower level code required for tiny processors supported by FastLED is replaced with aliases to C functions for the RaspBerry PI.

Our goal is to port several INO script examples from the FastLED web site to sample applications to provide a fairly good coverage of the
interesting functionality that is provided by FastLED.

## Content

### rpi-ws281x-rainbow

This folder contains code for a simple demonstration of how to use the rpi_ws281x library to drive an LED matrix.
The sample application assigns some colours to a strip of LEDs that render a rainbow over twenty pixels and then
shift up and over the coloured pixels.

[rpi-ws281x-rainbow](rpi-ws281x-rainbow)


## Circuit

This diagram shows a representation of a circuit connecting one LED strip to a RaspBerry PI 4, a 5V power supply and
using a [74AHCT125 - Quad Level-Shifter (3V to 5V)](https://www.adafruit.com/product/1787) from AdaFruit as a level inverter
to scale up the 3.3V voltage from the SPI0 MOSI output to the 5V required by the LED strip.

![circuit diagram connecting RaspBerry PI 4, 74AHCT125 chip, 5V power supply and LED strip](./led-strip-wiring.png)

