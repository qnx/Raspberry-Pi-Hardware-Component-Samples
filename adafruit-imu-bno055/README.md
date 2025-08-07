# Adafruit BNO055 9-DOF IMU for QNX
 
This repository provides example code in C and Python for interfacing with the Adafruit BNO055 9-DOF Absolute Orientation Sensor on the QNX SDP 8.0 RTOS.
 
The code is designed to initialize the sensor, manage a calibration process, and continuously read fused orientation data (Euler angles, Quaternions) as well as raw sensor data.

 > **Important:**  
> The implementation requires root privileges to access the I²C bus. The code uses QNX’s I²C API (via the `rpi_i2c` library).  

***
 
## Table of Contents
 
- [Overview](#overview)
- [Key Features](#key-features)
- [Hardware Setup](#hardware-setup)
- [Software Requirements](#software-requirements)
- [How to Build and Run (C)](#how-to-build-and-run-c)
- [How to Run (Python)](#how-to-run-python)
- [Calibration Guide](#calibration-guide)
  - [How It Works](#how-it-works)
  - [How to Perform Manual Calibration](#how-to-perform-manual-calibration)
- [Schematic](#schematic)
 
***
 
## Overview
 
The Adafruit BNO055 is a 9-DOF (Degrees of Freedom) sensor that combines an accelerometer, gyroscope, and magnetometer with an ARM Cortex-M0+ microcontroller. The sensor's onboard fusion algorithm processes data from all three sensors to provide absolute orientation.
 
This project provides the necessary tools to communicate with the BNO055 over an I²C bus on a Raspberry Pi running QNX. The primary C implementation demonstrates how to:
1.  **Initialize the sensor** and verify its identity by checking for the correct chip ID (`0xA0`).
2.  **Manage calibration data** by saving it to and loading it from a file.
3.  **Perform a guided manual calibration** if no existing calibration data is found or if forced by the user.
4.  **Continuously read** a wide range of sensor outputs, including Euler angles, quaternions, linear acceleration, and gravity vectors.
 
***
 
## Key Features
 
* **Calibration System:** Automatically saves and loads sensor calibration profiles to a `bno055_calib.dat` file, avoiding the need to re-calibrate on every power-up.
* **NDOF Fusion Mode:** Operates the sensor in the advanced **NDOF (Nine Degrees of Freedom)** fusion mode for stable absolute orientation data.
* **Data Output:** Reads and displays all major sensor outputs:
    * Fused Euler Angles (Heading, Roll, Pitch)
    * Fused Quaternions
    * Raw Accelerometer & Gyroscope Data
    * Raw Magnetometer Data
    * Linear Acceleration & Gravity Vectors
 
***
 
## Hardware Setup
 
To connect the Adafruit BNO055 sensor to a Raspberry Pi, wire the pins as follows. This setup uses the I²C bus for communication.
 
* **VIN:** Connect to a **3.3V** pin on the Raspberry Pi (e.g., Pin 1).
* **GND:** Connect to a **Ground** pin on the Raspberry Pi (e.g., Pin 6).
* **SCL (Clock):** Connect to the Raspberry Pi's **SCL** pin (GPIO 3, Pin 5).
* **SDA (Data):** Connect to the Raspberry Pi's **SDA** pin (GPIO 2, Pin 3).
* **RST:** (Optional) Can be connected to GPIO 27 (Pin 13). The example does not use the reset logic.
* **INT:** (Optional) Can be connected to GPIO 22 (Pin 15). The example does not use interrupts.
 
The default I²C address used in the code is **`0x28`**.
 
***
 
## Software Requirements
 
* **Operating System:** QNX SDP 8.0.
* **Compiler:** `qcc` compiler for `aarch64le` architecture.
* **Libraries:** The custom `rpi_i2c` library.
* **Privileges:** The application requires **root** privileges to access the low-level I²C hardware bus.
 
***
 
## How to Build and Run - C 
 
1.  **Navigate to the C Directory**
    Open a terminal and change to the directory `/adafruit-imu-bno055/c/`.

2.  **Compile the Code**
    You can compile manually using `qcc`:
    ```bash
    make rebuild
    ```

3. Transfer the compiled binary `imu-bno055` from `/adafruit-imu-bno055/c/build/aarch64le-debug/` to the target.

 
4.  **Run the Application on the Raspberry Pi**
     > **Important:** As the program requires direct hardware access, you must run it with root privileges.
 
    * **Normal Mode (Loads existing calibration):**
        ```bash
        ./imu-bno055
        ```
        The application will first look for `bno055_calib.dat`. If found, it loads the data to the sensor and begins reading data immediately.
 
    * **Forced Calibration Mode:**
        To start a new manual calibration session, run the program with the `--calibrate` flag.
        ```bash
        ./imu-bno055 --calibrate
        ```
        This will ignore any existing `.dat` file and initiate the guided calibration process.
 
***
 
## How to Run - Python
 
1.  **Navigate to the Python Directory**
    Change to the directory containing `/adafruit-imu-bno055/python/`.

2. Transfer the python file `imu-bno055.py` to the target.
 
 
3.  **Run the Script**
    The script must be run with root privileges to access the I²C hardware bus. It can be run in two modes:
 
    * **Normal Mode (Loads Existing Calibration):**
        When run without any arguments, the script will automatically look for the `bno055_calib.dat` file. If the file exists, it will load the data directly to the sensor and begin streaming data.
        ```bash
        python3 imu-bno055.py
        ```
 
    * **Forced Calibration Mode:**
        To start a new manual calibration session, run the script with the `--calibrate` flag. This will ignore any existing calibration file and begin the guided process.
        ```bash
        python3 imu-bno055.py --calibrate
        ```
        Once calibration is complete, the script will save the new data to `bno055_calib.dat` for future use.
 
***
 
***
 
## Calibration Guide
 
Proper calibration is needed for the BNO055 sensor to provide accurate absolute orientation data.
 
### How It Works
 
The BNO055 has internal registers that store offset and radius data for the accelerometer, gyroscope, and magnetometer. A well-calibrated sensor produces minimal drift and accurate heading.
 
* **The `bno055_calib.dat` File:** This program simplifies calibration by treating it as a one-time task. After a successful manual calibration, the application reads 22 bytes of calibration data from the sensor's registers and saves them into the `bno055_calib.dat` file. On subsequent runs, the program checks for this file, loads its contents back into the sensor, and bypasses the manual process entirely. This ensures your sensor is ready to use instantly.
 
* **Calibration Status:** The sensor provides real-time feedback on its calibration status. Each component (System, Gyro, Accelerometer, Magnetometer) is rated on a scale from 0 to 3, where `0` is uncalibrated and `3` is fully calibrated. The goal of the manual process is to get the Gyro, Accelerometer, and Magnetometer to a status of **3**. Once they are stable, the overall System (SYS) fusion algorithm will also reach level 3.
 
### How to Perform Manual Calibration
 
If you run the program with the `--calibrate` flag or if the `bno055_calib.dat` file is missing, the following guided process will begin.
 
You will see this output:
 
   ```bash
    Calibration Status: SYS=0, GYR=0, ACC=0, MAG=0
   ```
   
To get each component to a status of `3`, follow these steps:
 
1.  **Calibrate the Gyroscope (GYR):**
    * **Action:** Place the sensor on a flat, motionless surface.
    * **Result:** The `GYR` status should quickly change to `3`.
 
2.  **Calibrate the Accelerometer (ACC):**
    * **Action:** Slowly move the sensor into approximately six different stable orientations.
    * **Result:** The `ACC` status will change to `3`.
 
3.  **Calibrate the Magnetometer (MAG):**
    * **Action:** Slowly draw a large figure-8 pattern in the air with the sensor.
    * **Result:** The `MAG` status will change to `3`.
 
As you perform these actions, the status line will update continuously. Once `GYR`, `ACC`, and `MAG` all report `3`, the program will detect this, save the calibration data, and proceed to continuous data reading.
 
   ```bash
    Calibration Status: SYS=3, GYR=3, ACC=3, MAG=3
    Full System Calibration Complete!
    Successfully saved calibration data to bno055_calib.dat
   ```

***
 
## Schematic
 
A wiring diagram is placed in the `schematic/` directory of this repo. This diagram shows the pin connections between the Adafruit BNO055 and the Raspberry Pi.
 
 