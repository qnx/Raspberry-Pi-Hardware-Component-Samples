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

@file imu-icm-20948.py
@brief SparkFun ICM‑20948 9DoF IMU Example for QNX.
 
This example demonstrates how to interface with the SparkFun Qwiic ICM‑20948
9DoF IMU over I2C on a QNX system using Python's smbus module. It:
  - Initializes the sensor (wakes it and configures the accelerometer and gyroscope).
  - Reads the WHO_AM_I register (expected value: 0xEA).
  - Calibrates the sensor while it is stationary (computes offsets).
  - Continuously reads raw sensor data, subtracts the offsets, converts readings
    to physical units (acceleration in m/s² and gyroscope in °/s), applies an
    exponential moving average filter, and measures the time taken for each read.
 
Wiring (Raspberry Pi header mapping):
  - SDA: Connect to the IMU SDA line.
  - SCL: Connect to the IMU SCL line.
  - 3.3V: Connect to the 3.3V supply.
  - GND: Connect to Ground.
 
Sensitivities:
  - Accelerometer: ±4g, ~8192 LSB/g (converted to m/s² using 1g = 9.80665 m/s²).
  - Gyroscope: ±500 dps, ~65.5 LSB/(°/s).
"""
 
import smbus
import time
import sys
 
## --------------------------------------------------------------------------
## Definitions
## --------------------------------------------------------------------------
I2C_BUS = 1                     # I2C bus number
ICM20948_ADDR = 0x68            # ICM‑20948 I2C address (use 0x69 when AD0 is high or else 0x68 when AD0 is low)
 
REG_BANK_SEL = 0x7F           # Register bank selection register
 
# Register bank values
BANK_0 = 0x00
BANK_2 = 0x20
 
# BANK_0 registers
WHO_AM_I   = 0x00             # WHO_AM_I register; expected value: 0xEA
PWR_MGMT_1 = 0x06             # Power management 1
PWR_MGMT_2 = 0x07             # Power management 2
 
# BANK_2 registers (for sensor configuration)
ACCEL_CONFIG  = 0x14          # Accelerometer configuration register
GYRO_CONFIG_1 = 0x01          # Gyroscope configuration register
 
# Data registers in BANK_0 (starting at ACCEL_XOUT_H)
ACCEL_XOUT_H = 0x2D           # Accelerometer X high byte
 
# Conversion factors
ACCEL_SENSITIVITY = 8192.0    # LSB per g for ±4g range
GYRO_SENSITIVITY  = 65.5      # LSB per °/s for ±500 dps
 
CALIBRATION_SAMPLES = 100     # Number of samples for calibration
ALPHA = 0.1                   # Low-pass filter constant (0 < ALPHA <= 1; higher alpha means less smoothing)
LOOP_DELAY_US = 20000         # Loop delay in microseconds (~20 ms, ~50 Hz)
 
## --------------------------------------------------------------------------
## Create SMBus Object
## --------------------------------------------------------------------------
bus = smbus.SMBus(I2C_BUS)
 
## --------------------------------------------------------------------------
## Helper Function: read_block_data
## --------------------------------------------------------------------------
def read_block_data(addr: int, reg: int, length: int) -> list:
    """
    @brief Reads a block of bytes by sequentially reading each byte.
    
    Since the Python smbus module in this environment lacks a read_block_data method,
    this function reads each byte one-by-one starting from the specified register.
    
    @param addr I2C device address.
    @param reg The starting register address.
    @param length Number of bytes to read.
    @return list List of read byte values, or None if an error occurs.
    """
    block = []
    try:
        for i in range(length):
            val = bus.read_byte_data(addr, reg + i)
            block.append(val)
        return block
    except Exception as e:
        print(f"Error in read_block_data at reg 0x{reg:X}: {e}", file=sys.stderr)
        return None
 
## --------------------------------------------------------------------------
## Helper Functions for Register Read/Write
## --------------------------------------------------------------------------
def icm20948_select_bank(bank: int) -> int:
    """
    @brief Selects the given register bank.
    
    @param bank The register bank to select (e.g. BANK_0 or BANK_2).
    @return int 0 on success, or a nonzero error code.
    """
    try:
        bus.write_byte_data(ICM20948_ADDR, REG_BANK_SEL, bank)
        return 0
    except Exception as e:
        print(f"Error in icm20948_select_bank: {e}", file=sys.stderr)
        return 1
 
def icm20948_write_reg(reg: int, data: int) -> int:
    """
    @brief Writes a single byte to a specified register.
    
    @param reg The register address.
    @param data The data byte to write.
    @return int 0 on success, or a nonzero error code.
    """
    try:
        bus.write_byte_data(ICM20948_ADDR, reg, data)
        return 0
    except Exception as e:
        print(f"Error in icm20948_write_reg: {e}", file=sys.stderr)
        return 1
 
def icm20948_read_reg(reg: int) -> (int, int):
    """
    @brief Reads a single byte from a specified register.
    
    @param reg The register address.
    @return tuple (error, data) where error is 0 on success.
    """
    try:
        data = bus.read_byte_data(ICM20948_ADDR, reg)
        return 0, data
    except Exception as e:
        print(f"Error in icm20948_read_reg: {e}", file=sys.stderr)
        return 1, 0
 
## --------------------------------------------------------------------------
## Sensor Initialization
## --------------------------------------------------------------------------
def icm20948_init() -> int:
    """
    @brief Initializes the ICM‑20948 sensor.
    
    This function performs the following:
      - Switches to BANK_0 and reads the WHO_AM_I register.
      - Wakes the device (writes to PWR_MGMT_1 and PWR_MGMT_2).
      - Switches to BANK_2 to configure the accelerometer (±4g) and gyroscope (±500 dps).
      - Switches back to BANK_0 for normal operation.
    
    @return int 0 on success, or a nonzero error code.
    """
    rc = icm20948_select_bank(BANK_0)
    if rc != 0:
        print("ERROR: Could not switch to BANK_0", file=sys.stderr)
        return rc
    rc, whoami = icm20948_read_reg(WHO_AM_I)
    if rc != 0:
        print("ERROR: Could not read WHO_AM_I", file=sys.stderr)
        return rc
    print(f"ICM‑20948 WHO_AM_I = 0x{whoami:02X} (expected 0xEA)")
    
    rc = icm20948_write_reg(PWR_MGMT_1, 0x01)
    if rc != 0:
        print("ERROR: Could not write PWR_MGMT_1", file=sys.stderr)
        return rc
    rc = icm20948_write_reg(PWR_MGMT_2, 0x00)
    if rc != 0:
        print("ERROR: Could not write PWR_MGMT_2", file=sys.stderr)
        return rc
    time.sleep(0.01)  # 10 ms stabilization
    
    rc = icm20948_select_bank(BANK_2)
    if rc != 0:
        print("ERROR: Could not switch to BANK_2", file=sys.stderr)
        return rc
    rc = icm20948_write_reg(ACCEL_CONFIG, 0x01)
    if rc != 0:
        print("ERROR: Could not write ACCEL_CONFIG", file=sys.stderr)
        return rc
    rc = icm20948_write_reg(GYRO_CONFIG_1, 0x01)
    if rc != 0:
        print("ERROR: Could not write GYRO_CONFIG_1", file=sys.stderr)
        return rc
 
    rc = icm20948_select_bank(BANK_0)
    if rc != 0:
        print("ERROR: Could not switch back to BANK_0", file=sys.stderr)
        return rc
    print("ICM‑20948 initialization complete.")
    return 0
 
## --------------------------------------------------------------------------
## Sensor Data Reading
## --------------------------------------------------------------------------
def icm20948_read_sensors() -> (int, dict):
    """
    @brief Reads 12 consecutive bytes of raw sensor data.
    
    This function reads:
      - 6 bytes for the accelerometer (X, Y, Z: high and low bytes)
      - 6 bytes for the gyroscope (X, Y, Z: high and low bytes)
    starting at ACCEL_XOUT_H.
    
    @return tuple (error, data) where data is a dictionary with keys:
            'ax', 'ay', 'az', 'gx', 'gy', 'gz' (all as signed 16-bit integers).
    """
    raw = read_block_data(ICM20948_ADDR, ACCEL_XOUT_H, 12)
    if raw is None or len(raw) < 12:
        return 1, {}
    
    def to_int16(val):
        return val - 65536 if val > 32767 else val
 
    data = {
        'ax': to_int16((raw[0] << 8) | raw[1]),
        'ay': to_int16((raw[2] << 8) | raw[3]),
        'az': to_int16((raw[4] << 8) | raw[5]),
        'gx': to_int16((raw[6] << 8) | raw[7]),
        'gy': to_int16((raw[8] << 8) | raw[9]),
        'gz': to_int16((raw[10] << 8) | raw[11])
    }
    return 0, data
 
## --------------------------------------------------------------------------
## Calibration Routine
## --------------------------------------------------------------------------
def calibrate_imu(num_samples: int) -> (int, dict):
    """
    @brief Calibrates the IMU by averaging a set number of samples.
    
    This function computes offsets (in physical units) for both the accelerometer
    and gyroscope by taking num_samples while the sensor is stationary.
    The computed offsets are then used to correct subsequent readings.
    
    @param num_samples Number of samples to average.
    @return tuple (error, offsets) where offsets is a dictionary with keys:
            'accel_offset_x', 'accel_offset_y', 'accel_offset_z' (in g) and
            'gyro_offset_x', 'gyro_offset_y', 'gyro_offset_z' (in °/s).
    """
    sum_ax = sum_ay = sum_az = 0.0
    sum_gx = sum_gy = sum_gz = 0.0
 
    print("Calibrating IMU... please keep the sensor completely still.")
    for i in range(num_samples):
        rc, sensor_data = icm20948_read_sensors()
        if rc != 0:
            print(f"Calibration read error on sample {i}", file=sys.stderr)
            return rc, {}
        # Convert raw values to physical units.
        ax = sensor_data['ax'] / ACCEL_SENSITIVITY
        ay = sensor_data['ay'] / ACCEL_SENSITIVITY
        az = sensor_data['az'] / ACCEL_SENSITIVITY
        gx = sensor_data['gx'] / GYRO_SENSITIVITY
        gy = sensor_data['gy'] / GYRO_SENSITIVITY
        gz = sensor_data['gz'] / GYRO_SENSITIVITY
 
        sum_ax += ax
        sum_ay += ay
        sum_az += az
        sum_gx += gx
        sum_gy += gy
        sum_gz += gz
        time.sleep(0.005)  # 5 ms delay between samples
 
    offsets = {
        'accel_offset_x': sum_ax / num_samples,
        'accel_offset_y': sum_ay / num_samples,
        'accel_offset_z': sum_az / num_samples,
        'gyro_offset_x':  sum_gx / num_samples,
        'gyro_offset_y':  sum_gy / num_samples,
        'gyro_offset_z':  sum_gz / num_samples
    }
    print("Calibration complete:")
    print("  Accel offsets: X={:+7.3f} m/s^2, Y={:+7.3f} m/s^2, Z={:+7.3f} m/s^2".format(
          offsets['accel_offset_x'] * 9.80665,
          offsets['accel_offset_y'] * 9.80665,
          offsets['accel_offset_z'] * 9.80665))
    print("  Gyro offsets:  X={:+7.2f} dps, Y={:+7.2f} dps, Z={:+7.2f} dps".format(
          offsets['gyro_offset_x'],
          offsets['gyro_offset_y'],
          offsets['gyro_offset_z']))
    return 0, offsets
 
## --------------------------------------------------------------------------
## Low-Pass Filtering (Exponential Moving Average)
## --------------------------------------------------------------------------
def filter_value(prev: float, new_val: float, alpha: float) -> float:
    """
    @brief Updates a filtered value using an exponential moving average.
    
    @param prev The previous filtered value.
    @param new_val The new measurement.
    @param alpha Filter constant (0 < alpha <= 1).
    @return float The updated filtered value.
    """
    return alpha * new_val + (1.0 - alpha) * prev
 
## --------------------------------------------------------------------------
## Main Application Loop
## --------------------------------------------------------------------------
def main():
    """
    @brief Main function.
    
    The main loop performs the following:
      - Initializes the sensor.
      - Calibrates the sensor to determine offsets.
      - Continuously reads sensor data, subtracts offsets, converts raw values
        to physical units (acceleration in m/s² and gyroscope in °/s), applies an
        exponential moving average filter, and measures the read time.
    
    The formatted output displays the elapsed time in milliseconds and the filtered
    sensor values with fixed width and sign.
    """
    
    print("=== SparkFun ICM‑20948 Demo (IMU) ===");
    
    rc = icm20948_init()
    if rc != 0:
        print("ERROR: icm20948_init() failed.", file=sys.stderr)
        sys.exit(1)
    
    rc, offsets = calibrate_imu(CALIBRATION_SAMPLES)
    if rc != 0:
        print("ERROR: Calibration failed.", file=sys.stderr)
        sys.exit(1)
    
    # Initialize filtered values.
    filt_ax = filt_ay = filt_az = 0.0
    filt_gx = filt_gy = filt_gz = 0.0
    first_reading = True
 
    print("Entering main loop. Press Ctrl+C to exit.")
    while True:
        t_start = time.perf_counter()
        rc, sensor_data = icm20948_read_sensors()
        t_end = time.perf_counter()
        if rc != 0:
            print("ERROR: Could not read sensor data", file=sys.stderr)
        else:
            # Calculate elapsed time in milliseconds.
            elapsed_ms = (t_end - t_start) * 1000.0
            
            # Convert raw readings to physical units.
            # Accelerometer: from g to m/s².
            curr_ax = (sensor_data['ax'] / ACCEL_SENSITIVITY) - offsets['accel_offset_x']
            curr_ay = (sensor_data['ay'] / ACCEL_SENSITIVITY) - offsets['accel_offset_y']
            curr_az = (sensor_data['az'] / ACCEL_SENSITIVITY) - offsets['accel_offset_z']
            curr_gx = (sensor_data['gx'] / GYRO_SENSITIVITY) - offsets['gyro_offset_x']
            curr_gy = (sensor_data['gy'] / GYRO_SENSITIVITY) - offsets['gyro_offset_y']
            curr_gz = (sensor_data['gz'] / GYRO_SENSITIVITY) - offsets['gyro_offset_z']
            
            # Initialize filtered values on first reading.
            if first_reading:
                filt_ax, filt_ay, filt_az = curr_ax, curr_ay, curr_az
                filt_gx, filt_gy, filt_gz = curr_gx, curr_gy, curr_gz
                first_reading = False
            else:
                filt_ax = filter_value(filt_ax, curr_ax, ALPHA)
                filt_ay = filter_value(filt_ay, curr_ay, ALPHA)
                filt_az = filter_value(filt_az, curr_az, ALPHA)
                filt_gx = filter_value(filt_gx, curr_gx, ALPHA)
                filt_gy = filter_value(filt_gy, curr_gy, ALPHA)
                filt_gz = filter_value(filt_gz, curr_gz, ALPHA)
                
            # Print the time (in ms) and filtered sensor data.
            print("Time: {:5.2f} ms | ACCEL: X={:7.3f} m/s^2, Y={:7.3f} m/s^2, Z={:7.3f} m/s^2 | "
                  "GYRO: X={:6.2f} dps, Y={:6.2f} dps, Z={:6.2f} dps".format(
                      elapsed_ms,
                      filt_ax * 9.80665, filt_ay * 9.80665, filt_az * 9.80665,
                      filt_gx, filt_gy, filt_gz))
        time.sleep(LOOP_DELAY_US / 1e6)
 
if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\nExiting IMU demo.")
        sys.exit(0)
