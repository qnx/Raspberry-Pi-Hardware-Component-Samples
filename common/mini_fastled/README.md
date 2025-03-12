# libmini_fastled

## Overview

The mini_fastled library is based on the well known [FastLED](https://fastled.io/) library, but is not a complete port, at least not yet. The current approach to the port was inspired by another attempted port to Raspberry Pi found at [https://github.com/apleschu/FastLED-Pi](https://github.com/apleschu/FastLED-Pi) which leveraged the Linux rpi_ws281x library as well, but the approach we used for mini_fastled differs in many ways.

The QNX port has some significant differences from the original library:

- The QNX port is based on a C API, instead of C++, but the API is meant to be similar in some ways to the INO script APIs for FastLED, so that translation from INO script to C code is not too onerous.
- The library is layered on [librpi_ws281x](../common/librpi_ws281x), so certain data types are matched under the hood to that library's data types.
- Some of the lower level code required for tiny processors supported by FastLED is replaced with aliases to C functions for the Raspberry Pi.

Our goal is to port several INO script examples from the FastLED web site to sample applications to provide a fairly good coverage of the
interesting functionality that is provided by FastLED.

## APIs

This library consists of many categories of APIs.  Documentation of the APIs is still in progress so highlights of the different APIs will be added here as soon as possible.

In the meantime, please consult the available sample applications [here](../led-strip-matrix/mini_fastled) for more info about how to use the APIs currently available.

---
See [mini_fastled.h](public/mini_fastled.h) for more details about the higher level APIs and which headers to check next for lower level APIS.
