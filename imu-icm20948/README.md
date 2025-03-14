# SparkFun ICM‑20948 9DoF IMU for QNX
 
This repository contains sample code (in both C and Python) for interfacing with the SparkFun Qwiic ICM‑20948 9DoF IMU on QNX. Originally developed for the Trilobot platform, the code can be adapted for use on other hardware setups.
 
> **Important:**  
> Both implementations require root privileges to access the I²C bus. The code uses QNX’s I²C API (via the `rpi_i2c` library).  
> **Note on I²C Address:** When using the Qwiic cable, the wiring may pull the sensor’s AD0 pin low—resulting in an I²C address of **0x68** rather than the default **0x69**. Always verify the sensor’s WHO_AM_I register (expected value: 0xEA) to ensure proper communication.
 
## Table of Contents
 
- [Overview](#overview)
- [Hardware Setup](#hardware-setup)
  - [Trilobot Setup (Qwiic Connector)](#trilobot-setup-qwiic-connector)
  - [Direct Raspberry Pi Connection](#direct-raspberry-pi-connection)
  - [Breadboard/Other Platforms](#breadboardother-platforms)
- [Qwiic Cable Considerations](#qwiic-cable-considerations)
- [Schematic](#schematic)
- [Software Requirements](#software-requirements)
- [Build the Code](#build-the-code)
 
## Overview
 
The ICM‑20948 is a 9DoF inertial measurement unit that combines a 3-axis accelerometer, 3-axis gyroscope, and (optionally) a magnetometer. This demo code performs the following tasks:
 
1. **Sensor Initialization:**  
   The code wakes the sensor, configures the accelerometer (e.g., ±4g) and gyroscope (e.g., ±500°/s), and verifies communication by reading the WHO_AM_I register.
 
2. **Calibration:**  
   When the sensor is stationary, the software computes calibration offsets for the accelerometer and gyroscope.
 
3. **Data Acquisition & Processing:**  
   The code continuously reads raw sensor data, subtracts calibration offsets, converts the data into physical units (m/s² and °/s), applies an exponential moving average filter, and measures the read time.
 
## Hardware Setup
 
### Trilobot Setup (Qwiic Connector)
 
On the Trilobot platform, the ICM‑20948 is connected via the Qwiic connector. When connected using the Qwiic cable, the internal wiring typically pulls the AD0 pin low—resulting in an I²C address of **0x68** (instead of **0x69**). Verify the address by checking the WHO_AM_I register.
 
Typical connections for the Trilobot are:
 
- **SDA:** Connect to the sensor’s SDA line on the Qwiic connector.
- **SCL:** Connect to the sensor’s SCL line on the Qwiic connector.
- **3.3V:** Connect to the 3.3V supply.
- **GND:** Connect to Ground.
 
### Direct Raspberry Pi Connection
 
You can also connect the ICM‑20948 directly to a Raspberry Pi using its GPIO pins. In this configuration, the sensor is connected as follows:
 
- **SDA (Data):** Connect to GPIO Pin 2 (SDA).
- **SCL (Clock):** Connect to GPIO Pin 3 (SCL).
- **3.3V (VCC):** Connect to the Pi’s 3.3V supply.
- **GND:** Connect to the Pi’s Ground.
 
> **Note:**  
   If you use the Qwiic cable with the Pi, the wiring may pull the AD0 pin low, so the sensor’s I²C address will be **0x68**.
 
### Breadboard / Other Platforms
 
For use on a breadboard or other hardware setups:
 
- **Power:**  
  Provide a stable 3.3V supply to the sensor and ensure proper grounding.
 
- **I²C Connections:**  
  - **SDA:** Connect to the appropriate I²C data line.
  - **SCL:** Connect to the appropriate I²C clock line.
 
- **Address Configuration:**  
  The sensor’s I²C address is determined by the state of the AD0 pin:
  - **AD0 Low:** Address is **0x68**.
  - **AD0 High:** Address is **0x69**.

 
## Schematic
 
A schematic for the Trilobot wiring is available in the `schematic/` folder. This schematic shows the connections for the sensor on the Raspberry Pi and can be used as a reference when setting up your own circuit.
 
## Software Requirements
 
- **QNX Operating System 8.0 or later**
- **I²C API:**  
  The demo code uses QNX’s I²C API (the `rpi_i2c` library) available in the `common/rpi_i2c` folder.
- **Root Privileges:**  
  Root access is required to access the I²C bus.

 
## Build the Code
 
From the `imu-icm-20948/c` directory, run:
 
```bash
make all
```
 
