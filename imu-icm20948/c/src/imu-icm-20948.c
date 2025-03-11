/**
* Copyright (c) 2025, BlackBerry Limited. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* @file imu-icm-20948.c
* @brief SparkFun ICM‑20948 9DoF IMU Example for QNX.
*
* This example demonstrates how to interface with the SparkFun Qwiic ICM‑20948
* 9DoF IMU over I2C on a QNX system. It:
*   - Initializes the sensor (wakes it and configures the accelerometer and gyroscope).
*   - Reads the WHO_AM_I register (expected value: 0xEA).
*   - Calibrates the sensor while it is stationary (computes offsets).
*   - Continuously reads raw sensor data, subtracts the offsets, converts readings
*     to physical units (m/s² for acceleration, °/s for gyroscope), applies an
*     exponential moving average filter, and measures the time taken for each read.
*
* Wiring: (Raspberry Pi header mapping):
*   - SDA: Connect to the IMU SDA line.
*   - SCL: Connect to the IMU SCL line.
*   - 3.3V: Connect to the 3.3V supply.
*   - GND: Connect to Ground.
*
* Sensitivities:
*   - Accelerometer: ±4g, ~8192 LSB/g (converted to m/s² with 1 g = 9.80665 m/s²).
*   - Gyroscope: ±500 dps, ~65.5 LSB/(°/s).
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "rpi_i2c.h"  // I2C API functions
 
/* --------------------------------------------------------------------------
* Definitions
* ------------------------------------------------------------------------*/
// I2C bus 
#define I2C_BUS         1
 
// ICM‑20948 I2C address (use 0x69 when AD0 is high or else 0x68 when AD0 is low)
#define ICM20948_ADDR   0x69
 
// Register bank selection register
#define REG_BANK_SEL    0x7F
 
// Register bank values
#define BANK_0          0x00
#define BANK_2          0x20
 
// Bank 0 registers
#define WHO_AM_I        0x00    ///< WHO_AM_I register; expected value: 0xEA
#define PWR_MGMT_1      0x06    ///< Power management 1
#define PWR_MGMT_2      0x07    ///< Power management 2
 
// Bank 2 registers (for sensor configuration)
#define ACCEL_CONFIG    0x14    ///< Accelerometer configuration register
#define GYRO_CONFIG_1   0x01    ///< Gyroscope configuration register
 
// Data registers in Bank 0 (starting at ACCEL_XOUT_H)
#define ACCEL_XOUT_H    0x2D    ///< Accelerometer X high byte
 
// Conversion factors (from datasheet with our chosen configuration)
#define ACCEL_SENSITIVITY   8192.0   ///< LSB per g for ±4g range
#define GYRO_SENSITIVITY    65.5     ///< LSB per °/s for ±500 dps
 
// Number of samples to average for calibration
#define CALIBRATION_SAMPLES 100
 
// Low‑pass filter constant (0 < ALPHA <= 1; higher alpha means less smoothing)
#define ALPHA 0.1
 
// Loop delay in microseconds (e.g., 20000 us = 20 ms ~ 50 Hz update rate)
#define LOOP_DELAY_US 20000
 
/* --------------------------------------------------------------------------
* Helper Functions for Register Read/Write
* ------------------------------------------------------------------------*/
 
/**
* @brief Selects the given register bank.
*
* @param bank The register bank to select (e.g. BANK_0, BANK_2).
* @return int I2C_SUCCESS on success; error code otherwise.
*/
static int icm20948_select_bank(uint8_t bank)
{
    return smbus_write_byte_data(I2C_BUS, ICM20948_ADDR, REG_BANK_SEL, bank);
}
 
/**
* @brief Writes a single byte to a specified register.
*
* @param reg The register address.
* @param data The data byte to write.
* @return int I2C_SUCCESS on success; error code otherwise.
*/
static int icm20948_write_reg(uint8_t reg, uint8_t data)
{
    return smbus_write_byte_data(I2C_BUS, ICM20948_ADDR, reg, data);
}
 
/**
* @brief Reads a single byte from a specified register.
*
* @param reg The register address.
* @param data Pointer to store the read byte.
* @return int I2C_SUCCESS on success; error code otherwise.
*/
static int icm20948_read_reg(uint8_t reg, uint8_t *data)
{
    return smbus_read_byte_data(I2C_BUS, ICM20948_ADDR, reg, data);
}
 
/* --------------------------------------------------------------------------
* Sensor Initialization
* ------------------------------------------------------------------------*/
 
/**
* @brief Initializes the ICM‑20948 sensor.
*
* This function performs the following:
*   - Switches to BANK_0 and reads the WHO_AM_I register.
*   - Wakes the device (writes to PWR_MGMT_1 and PWR_MGMT_2).
*   - Switches to BANK_2 to configure the accelerometer (±4g) and gyroscope (±500 dps).
*   - Switches back to BANK_0 for normal operation.
*
* @return int I2C_SUCCESS on success; error code otherwise.
*/
static int icm20948_init(void)
{
    int rc;
    uint8_t whoami = 0;
 
    // Switch to BANK_0 and read WHO_AM_I
    rc = icm20948_select_bank(BANK_0);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: Could not switch to BANK_0\n");
        return rc;
    }
    rc = icm20948_read_reg(WHO_AM_I, &whoami);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: Could not read WHO_AM_I\n");
        return rc;
    }
    printf("ICM‑20948 WHO_AM_I = 0x%02X (expected 0xEA)\n", whoami);
 
    // Wake up the device and set auto clock
    rc = icm20948_write_reg(PWR_MGMT_1, 0x01);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: Could not write PWR_MGMT_1\n");
        return rc;
    }
    // Enable all sensors
    rc = icm20948_write_reg(PWR_MGMT_2, 0x00);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: Could not write PWR_MGMT_2\n");
        return rc;
    }
    usleep(10000);  // 10 ms delay for stabilization
 
    // Configure accelerometer and gyroscope
    rc = icm20948_select_bank(BANK_2);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: Could not switch to BANK_2\n");
        return rc;
    }
    // Set accelerometer to ±4g
    rc = icm20948_write_reg(ACCEL_CONFIG, 0x01);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: Could not write ACCEL_CONFIG\n");
        return rc;
    }
    // Set gyroscope to ±500 dps
    rc = icm20948_write_reg(GYRO_CONFIG_1, 0x01);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: Could not write GYRO_CONFIG_1\n");
        return rc;
    }
    // Switch back to BANK_0 for normal operation
    rc = icm20948_select_bank(BANK_0);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: Could not switch back to BANK_0\n");
        return rc;
    }
    printf("ICM‑20948 initialization complete.\n");
    return I2C_SUCCESS;
}
 
/* --------------------------------------------------------------------------
* Sensor Data Reading
* ------------------------------------------------------------------------*/
 
/**
* @brief Reads 12 consecutive bytes of raw sensor data.
*
* This function reads:
*   - 6 bytes for the accelerometer (X, Y, Z: high and low bytes)
*   - 6 bytes for the gyroscope (X, Y, Z: high and low bytes)
* starting at ACCEL_XOUT_H.
*
* @param ax Pointer to store accelerometer X reading.
* @param ay Pointer to store accelerometer Y reading.
* @param az Pointer to store accelerometer Z reading.
* @param gx Pointer to store gyroscope X reading.
* @param gy Pointer to store gyroscope Y reading.
* @param gz Pointer to store gyroscope Z reading.
* @return int I2C_SUCCESS on success; error code otherwise.
*/
static int icm20948_read_sensors(int16_t *ax, int16_t *ay, int16_t *az,
                                 int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t raw[12];
    int rc = smbus_read_block_data(I2C_BUS, ICM20948_ADDR, ACCEL_XOUT_H, raw, 12);
    if (rc != I2C_SUCCESS) {
        return rc;
    }
    *ax = (int16_t)((raw[0] << 8) | raw[1]);
    *ay = (int16_t)((raw[2] << 8) | raw[3]);
    *az = (int16_t)((raw[4] << 8) | raw[5]);
    *gx = (int16_t)((raw[6] << 8) | raw[7]);
    *gy = (int16_t)((raw[8] << 8) | raw[9]);
    *gz = (int16_t)((raw[10] << 8) | raw[11]);
    return I2C_SUCCESS;
}
 
/* --------------------------------------------------------------------------
* Calibration Routine
* ------------------------------------------------------------------------*/
 
/**
* @brief Calibrates the IMU by averaging a set number of samples.
*
* This function computes offsets (in physical units) for both the accelerometer
* and gyroscope by taking CALIBRATION_SAMPLES while the sensor is stationary.
* The computed offsets are then used to correct subsequent readings.
*
* @param num_samples Number of samples to average.
* @param accel_offset_x Pointer to store accelerometer X offset (in g).
* @param accel_offset_y Pointer to store accelerometer Y offset (in g).
* @param accel_offset_z Pointer to store accelerometer Z offset (in g).
* @param gyro_offset_x Pointer to store gyroscope X offset (in °/s).
* @param gyro_offset_y Pointer to store gyroscope Y offset (in °/s).
* @param gyro_offset_z Pointer to store gyroscope Z offset (in °/s).
* @return int I2C_SUCCESS on success; error code otherwise.
*/
static int calibrate_imu(int num_samples,
                         double *accel_offset_x, double *accel_offset_y, double *accel_offset_z,
                         double *gyro_offset_x, double *gyro_offset_y, double *gyro_offset_z)
{
    int rc;
    int16_t ax, ay, az, gx, gy, gz;
    double sum_ax = 0, sum_ay = 0, sum_az = 0;
    double sum_gx = 0, sum_gy = 0, sum_gz = 0;
 
    printf("Calibrating IMU... please keep the sensor completely still.\n");
    for (int i = 0; i < num_samples; i++) {
        rc = icm20948_read_sensors(&ax, &ay, &az, &gx, &gy, &gz);
        if (rc != I2C_SUCCESS) {
            fprintf(stderr, "Calibration read error on sample %d\n", i);
            return rc;
        }
        sum_ax += ax / ACCEL_SENSITIVITY;
        sum_ay += ay / ACCEL_SENSITIVITY;
        sum_az += az / ACCEL_SENSITIVITY;
        sum_gx += gx / GYRO_SENSITIVITY;
        sum_gy += gy / GYRO_SENSITIVITY;
        sum_gz += gz / GYRO_SENSITIVITY;
        usleep(5000); // 5 ms delay between samples
    }
    *accel_offset_x = sum_ax / num_samples;
    *accel_offset_y = sum_ay / num_samples;
    *accel_offset_z = sum_az / num_samples;
    *gyro_offset_x  = sum_gx / num_samples;
    *gyro_offset_y  = sum_gy / num_samples;
    *gyro_offset_z  = sum_gz / num_samples;
 
    printf("Calibration complete:\n");
    printf("  Accel offsets: X=%+7.3f m/s^2, Y=%+7.3f m/s^2, Z=%+7.3f m/s^2\n",
           *accel_offset_x * 9.80665, *accel_offset_y * 9.80665, *accel_offset_z * 9.80665);
    printf("  Gyro offsets:  X=%+7.2f dps, Y=%+7.2f dps, Z=%+7.2f dps\n",
           *gyro_offset_x, *gyro_offset_y, *gyro_offset_z);
    return I2C_SUCCESS;
}
 
/* --------------------------------------------------------------------------
* Low-Pass Filtering (Exponential Moving Average)
* ------------------------------------------------------------------------*/
 
/**
* @brief Updates a filtered value using an exponential moving average.
*
* @param prev The previous filtered value.
* @param new_val The new measurement.
* @param alpha Filter constant (0 < alpha <= 1).
* @return double The updated filtered value.
*/
static double filter(double prev, double new_val, double alpha)
{
    return alpha * new_val + (1.0 - alpha) * prev;
}

/* --------------------------------------------------------------------------
* Main Application Loop
* ------------------------------------------------------------------------*/
 
/**
* @brief Main function.
*
* The main loop performs the following:
*   - Initializes the sensor.
*   - Calibrates the sensor to determine offsets.
*   - Continuously reads sensor data, subtracts offsets, converts raw values
*     to physical units (acceleration in m/s^2 and gyroscope in °/s), applies
*     an exponential moving average filter, and measures the read time.
*
* @return int EXIT_SUCCESS on successful execution.
*/
int main(void)
{
    int rc;
    int16_t ax, ay, az, gx, gy, gz;
    double elapsed_us;
    struct timespec t_start, t_end;
 
    // Offsets determined during calibration (in physical units, where acceleration is in g)
    double accel_offset_x, accel_offset_y, accel_offset_z;
    double gyro_offset_x, gyro_offset_y, gyro_offset_z;
 
    // Filtered values for smoothing
    double filt_ax = 0, filt_ay = 0, filt_az = 0;
    double filt_gx = 0, filt_gy = 0, filt_gz = 0;
    int first_reading = 1; // Flag to initialize filtered values
 
    printf("=== SparkFun ICM‑20948 Demo (IMU) ===\n");
 
    // Initialize the sensor.
    rc = icm20948_init();
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: icm20948_init() failed.\n");
        return EXIT_FAILURE;
    }
    
    rc = calibrate_imu(CALIBRATION_SAMPLES,
                       &accel_offset_x, &accel_offset_y, &accel_offset_z,
                       &gyro_offset_x, &gyro_offset_y, &gyro_offset_z);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: Calibration failed.\n");
        return EXIT_FAILURE;
    }
 
    // Main loop: read sensor data, subtract offsets, convert to physical units,
    // apply filtering, and measure read time.
    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &t_start);
        rc = icm20948_read_sensors(&ax, &ay, &az, &gx, &gy, &gz);
        clock_gettime(CLOCK_MONOTONIC, &t_end);
        if (rc != I2C_SUCCESS) {
            fprintf(stderr, "ERROR: Could not read sensor data\n");
        } else {
            // Calculate elapsed time in microseconds and convert to milliseconds.
            elapsed_us = (t_end.tv_sec - t_start.tv_sec) * 1e6 +
                         (t_end.tv_nsec - t_start.tv_nsec) / 1000.0;
            double elapsed_ms = elapsed_us / 1000.0;
            
            // Convert raw readings to physical units and subtract calibration offsets.
            // Accelerometer: from g to m/s^2 (multiply by 9.80665).
            double curr_ax = (ax / ACCEL_SENSITIVITY) - accel_offset_x;
            double curr_ay = (ay / ACCEL_SENSITIVITY) - accel_offset_y;
            double curr_az = (az / ACCEL_SENSITIVITY) - accel_offset_z;
            double curr_gx = (gx / GYRO_SENSITIVITY) - gyro_offset_x;
            double curr_gy = (gy / GYRO_SENSITIVITY) - gyro_offset_y;
            double curr_gz = (gz / GYRO_SENSITIVITY) - gyro_offset_z;
 
            // Initialize filtered values on first reading.
            if (first_reading) {
                filt_ax = curr_ax;
                filt_ay = curr_ay;
                filt_az = curr_az;
                filt_gx = curr_gx;
                filt_gy = curr_gy;
                filt_gz = curr_gz;
                first_reading = 0;
            } else {
            // Apply exponential moving average filter.
                filt_ax = filter(filt_ax, curr_ax, ALPHA);
                filt_ay = filter(filt_ay, curr_ay, ALPHA);
                filt_az = filter(filt_az, curr_az, ALPHA);
                filt_gx = filter(filt_gx, curr_gx, ALPHA);
                filt_gy = filter(filt_gy, curr_gy, ALPHA);
                filt_gz = filter(filt_gz, curr_gz, ALPHA);
            }
 
            // Print the time (in ms) and filtered sensor data.
            printf("Time: %5.2f ms | ACCEL: X=%7.3f m/s^2, Y=%7.3f m/s^2, Z=%7.3f m/s^2 | "
                   "GYRO: X=%6.2f dps, Y=%6.2f dps, Z=%6.2f dps\n",
                   elapsed_ms,
                   filt_ax * 9.80665, filt_ay * 9.80665, filt_az * 9.80665,
                   filt_gx, filt_gy, filt_gz);
        }
        usleep(LOOP_DELAY_US);
    }
    return EXIT_SUCCESS;
}
 
