#!/usr/bin/env python3
"""
Copyright (c) 2025, BlackBerry Limited. All rights reserved.
 
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
 
     http://www.apache.org/licenses/LICENSE-2.0
 
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 
@file imu-bno055.py
@brief Adafruit BNO055 9-DOF IMU Example for QNX.
 
This example demonstrates how to interface with the Adafruit BNO055 Absolute
Orientation Sensor over I2C on a QNX system using Python's smbus module. It:
  - Initializes the sensor and verifies its chip ID (expected value: 0xA0).
  - Implements a calibration system:
    - Attempts to load existing calibration data from a file.
    - If no file exists or if `--calibrate` is passed as an argument, it
      initiates a guided, manual calibration process.
    - Saves the new calibration data to a file for future use.
  - Sets the sensor to NDOF (Nine Degrees of Freedom) fusion mode.
  - Continuously reads and prints all fused and raw sensor data.
 
Wiring (Raspberry Pi header mapping):
  - VIN: Connect to PIN 1 (3.3V Power).
  - GND: Connect to PIN 6 (Ground).
  - SCL: Connect to PIN 5 (GPIO 3).
  - SDA: Connect to PIN 3 (GPIO 2).
"""
 
import smbus
import time
import sys
import argparse
import os
 
## --------------------------------------------------------------------------
## Definitions
## --------------------------------------------------------------------------
BNO055_I2C_BUS          = 1
BNO055_ADDRESS          = 0x28
CALIBRATION_FILENAME    = "bno055_calib.dat"
 
# BNO055 Register Map
BNO055_CHIP_ID_ADDR          = 0x00
BNO055_OPR_MODE_ADDR         = 0x3D
BNO055_SYS_TRIGGER_ADDR      = 0x3F
BNO055_CALIB_STAT_ADDR       = 0x35
BNO055_ACC_OFFSET_X_LSB_ADDR = 0x55
CALIBRATION_DATA_SIZE        = 22
 
# Data Registers
BNO055_ACC_DATA_X_LSB_ADDR  = 0x08
BNO055_MAG_DATA_X_LSB_ADDR  = 0x0E
BNO055_GYR_DATA_X_LSB_ADDR  = 0x14
BNO055_EUL_HEADING_LSB_ADDR = 0x1A
BNO055_QUA_DATA_W_LSB_ADDR  = 0x20
BNO055_LIA_DATA_X_LSB_ADDR  = 0x28
BNO055_GRV_DATA_X_LSB_ADDR  = 0x2E
 
# BNO055 Operation Modes
BNO055_OPERATION_MODE_CONFIG = 0x00
BNO055_OPERATION_MODE_NDOF   = 0x0C

# Data Scaling Divisors (LSB to physical units)
BNO055_ACCEL_DIVISOR    = 100.0
BNO055_MAG_DIVISOR      = 16.0
BNO055_GYRO_DIVISOR     = 16.0
BNO055_EULER_DIVISOR    = 16.0
BNO055_QUAT_SCALE       = 1.0 / (1 << 14)
 
## --------------------------------------------------------------------------
## Create SMBus Object
## --------------------------------------------------------------------------
bus = smbus.SMBus(BNO055_I2C_BUS)
 
## --------------------------------------------------------------------------
## Helper Functions for Register Read/Write
## --------------------------------------------------------------------------
def _signed_int16(val: int) -> int:
    """Converts a 16-bit unsigned value to a signed integer."""
    return val - 65536 if val & 0x8000 else val
 
def bno055_set_mode(mode: int):
    """
    @brief Sets the operation mode of the BNO055.
    @param mode The desired operation mode (e.g., BNO055_OPERATION_MODE_CONFIG).
    """
    try:
        bus.write_byte_data(BNO055_ADDRESS, BNO055_OPR_MODE_ADDR, mode)
        time.sleep(0.02) # Wait for mode switch (datasheet recommends >19ms)
    except OSError as e:
        print(f"ERROR: Could not set mode: {e}", file=sys.stderr)
        sys.exit(1)
 
## --------------------------------------------------------------------------
## Sensor Initialization
## --------------------------------------------------------------------------
def bno055_init() -> int:
    """
    @brief Initializes the BNO055 sensor.
    
    This function performs the following:
      - Verifies the sensor's CHIP_ID.
      - Resets the sensor.
    
    @return int 0 on success, or a nonzero error code.
    """
    try:
        chip_id = bus.read_byte_data(BNO055_ADDRESS, BNO055_CHIP_ID_ADDR)
        if chip_id != 0xA0:
            print(f"ERROR: BNO055 not found at I2C address 0x{BNO055_ADDRESS:02X}. Chip ID: 0x{chip_id:02X}", file=sys.stderr)
            return 1
        print(f"BNO055 Chip ID: 0x{chip_id:02X}")
        
        # Reset the sensor
        bno055_set_mode(BNO055_OPERATION_MODE_CONFIG)
        bus.write_byte_data(BNO055_ADDRESS, BNO055_SYS_TRIGGER_ADDR, 0x20)
        time.sleep(0.7) # Wait for reset to complete
        return 0
    except OSError as e:
        print(f"ERROR: During sensor initialization: {e}", file=sys.stderr)
        return 1
 
## --------------------------------------------------------------------------
## Sensor Data Reading
## --------------------------------------------------------------------------
def bno055_read_vector(reg: int, divisor: float) -> tuple:
    """
    @brief Reads a 3-axis vector from the BNO055.
    @param reg The starting register address for the 3-axis data (6 bytes).
    @param divisor The scaling factor to convert the raw value to physical units.
    @return tuple A tuple (x, y, z) of the scaled vector data.
    """
    try:
        data = bus.read_i2c_block_data(BNO055_ADDRESS, reg, 6)
        x = _signed_int16(data[0] | (data[1] << 8)) / divisor
        y = _signed_int16(data[2] | (data[3] << 8)) / divisor
        z = _signed_int16(data[4] | (data[5] << 8)) / divisor
        return (x, y, z)
    except OSError:
        return (0, 0, 0)
 
def bno055_read_quaternion() -> tuple:
    """
    @brief Reads the quaternion data from the BNO055.
    @return tuple A tuple (w, x, y, z) of the quaternion data.
    """
    scale = 1.0 / (1 << 14)
    try:
        data = bus.read_i2c_block_data(BNO055_ADDRESS, BNO055_QUA_DATA_W_LSB_ADDR, 8)
        w = _signed_int16(data[0] | (data[1] << 8)) * scale
        x = _signed_int16(data[2] | (data[3] << 8)) * scale
        y = _signed_int16(data[4] | (data[5] << 8)) * scale
        z = _signed_int16(data[6] | (data[7] << 8)) * scale
        return (w, x, y, z)
    except OSError:
        return (0, 0, 0, 0)
 
def bno055_read_and_print_all_data():
    """
    @brief Reads all available sensor data and prints it to the console.
    """
    euler = bno055_read_vector(BNO055_EUL_HEADING_LSB_ADDR, BNO055_EULER_DIVISOR)
    accel = bno055_read_vector(BNO055_ACC_DATA_X_LSB_ADDR, BNO055_ACCEL_DIVISOR)
    mag = bno055_read_vector(BNO055_MAG_DATA_X_LSB_ADDR, BNO055_MAG_DIVISOR)
    gyro = bno055_read_vector(BNO055_GYR_DATA_X_LSB_ADDR, BNO055_GYRO_DIVISOR)
    lia = bno055_read_vector(BNO055_LIA_DATA_X_LSB_ADDR, BNO055_ACCEL_DIVISOR)
    grv = bno055_read_vector(BNO055_GRV_DATA_X_LSB_ADDR, BNO055_ACCEL_DIVISOR)
    quat = bno055_read_quaternion()
 
    print("--------------------------------------------------")
    print(f"Euler (H,R,P):        {euler[0]:7.2f}, {euler[1]:7.2f}, {euler[2]:7.2f} deg")
    print(f"Quaternion (W,X,Y,Z): {quat[0]:7.2f}, {quat[1]:7.2f}, {quat[2]:7.2f}, {quat[3]:7.2f}")
    print(f"Accelerometer (m/s^2):  X={accel[0]:7.2f}, Y={accel[1]:7.2f}, Z={accel[2]:7.2f}")
    print(f"Linear Accel (m/s^2):   X={lia[0]:7.2f}, Y={lia[1]:7.2f}, Z={lia[2]:7.2f}")
    print(f"Gravity Vector (m/s^2): X={grv[0]:7.2f}, Y={grv[1]:7.2f}, Z={grv[2]:7.2f}")
    print(f"Gyroscope (dps):        X={gyro[0]:7.2f}, Y={gyro[1]:7.2f}, Z={gyro[2]:7.2f}")
    print(f"Magnetometer (uT):      X={mag[0]:7.2f}, Y={mag[1]:7.2f}, Z={mag[2]:7.2f}")
 
## --------------------------------------------------------------------------
## Calibration Routine
## --------------------------------------------------------------------------
def bno055_perform_manual_calibration():
    """
    @brief Guides the user through a manual calibration process.
    """
    print("\n--- Starting Manual Calibration ---")
    print("Please perform the calibration dance until all components are 3:")
    print("1. Gyroscope (GYR): Place on a flat, stable surface.")
    print("2. Accelerometer (ACC): Slowly move to ~6 different stable orientations.")
    print("3. Magnetometer (MAG): Slowly make a large figure-8 motion.\n")
 
    while True:
        try:
            calib_stat = bus.read_byte_data(BNO055_ADDRESS, BNO055_CALIB_STAT_ADDR)
            sys_calib = (calib_stat >> 6) & 0x03
            gyro_calib = (calib_stat >> 4) & 0x03
            accel_calib = (calib_stat >> 2) & 0x03
            mag_calib = calib_stat & 0x03
 
            print(f"Calibration Status: SYS={sys_calib}, GYR={gyro_calib}, ACC={accel_calib}, MAG={mag_calib}", end='\r', flush=True)
 
            if gyro_calib == 3 and accel_calib == 3 and mag_calib == 3 and sys_calib == 3:
                print("\nFull System Calibration Complete!")
                break
            time.sleep(0.1)
        except OSError as e:
            print(f"\nError reading calibration status: {e}", file=sys.stderr)
            time.sleep(1)
 
def bno055_save_calibration(filename: str):
    """
    @brief Saves the sensor's calibration data to a file.
    @param filename The name of the file to save the data to.
    """
    print(f"Saving calibration data to {filename}...")
    bno055_set_mode(BNO055_OPERATION_MODE_CONFIG)
    try:
        calib_data = bus.read_i2c_block_data(BNO055_ADDRESS, BNO055_ACC_OFFSET_X_LSB_ADDR, CALIBRATION_DATA_SIZE)
        with open(filename, 'wb') as f:
            f.write(bytearray(calib_data))
        print("Successfully saved calibration data.")
    except (OSError, IOError) as e:
        print(f"Error saving calibration data: {e}", file=sys.stderr)
 
def bno055_load_calibration(filename: str) -> bool:
    """
    @brief Loads calibration data from a file and writes it to the sensor.
    @param filename The name of the file to load data from.
    @return bool True if loading was successful, False otherwise.
    """
    if not os.path.exists(filename):
        print("INFO: Calibration file not found. Starting manual calibration.")
        return False
    
    print(f"Loading calibration data from {filename}...")
    try:
        with open(filename, 'rb') as f:
            calib_data = list(f.read(CALIBRATION_DATA_SIZE))
        
        bno055_set_mode(BNO055_OPERATION_MODE_CONFIG)
        bus.write_i2c_block_data(BNO055_ADDRESS, BNO055_ACC_OFFSET_X_LSB_ADDR, calib_data)
        print("Successfully loaded calibration data.")
        return True
    except (OSError, IOError) as e:
        print(f"Error loading calibration data: {e}", file=sys.stderr)
        return False
 
## --------------------------------------------------------------------------
## Main Application Loop
## --------------------------------------------------------------------------
def main():
    """
    @brief Main function.
    
    The main loop performs the following:
      - Initializes the sensor.
      - Handles calibration loading or execution.
      - Enters a loop to continuously read and display sensor data.
    """
    parser = argparse.ArgumentParser(description="BNO055 IMU reader for QNX.")
    parser.add_argument('--calibrate', action='store_true', help='Force manual recalibration.')
    args = parser.parse_args()
 
    print("=== Adafruit BNO055 Demo ===")
 
    rc = bno055_init()
    if rc != 0:
        print("ERROR: bno055_init() failed.", file=sys.stderr)
        sys.exit(1)
 
   # Load calibration or perform manual calibration
    if args.calibrate or not bno055_load_calibration(CALIBRATION_FILENAME):
        bno055_set_mode(BNO055_OPERATION_MODE_NDOF)
        bno055_perform_manual_calibration()
        bno055_save_calibration(CALIBRATION_FILENAME)
 
    # Set final operation mode for data reading
    print("Setting final operation mode to NDOF...")
    bno055_set_mode(BNO055_OPERATION_MODE_NDOF)
    
    print("\n--- Starting Continuous Data Readout (Press Ctrl+C to exit) ---")
    while True:
        bno055_read_and_print_all_data()
        time.sleep(0.1) # ~10Hz
 
if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\nExiting BNO055 demo.")
        # The smbus object's destructor will handle closing the bus connection.
        sys.exit(0)
    except OSError as e:
        print(f"\nA fatal bus error occurred: {e}", file=sys.stderr)
        sys.exit(1)
 