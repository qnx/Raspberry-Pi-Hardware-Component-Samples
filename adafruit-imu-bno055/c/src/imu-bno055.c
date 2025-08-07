/**
 * Copyright (c) 2025, BlackBerry Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file imu-bno055.c
 * @brief Adafruit BNO055 9-DOF IMU Example for QNX.
 *
 * This example demonstrates how to interface with the Adafruit BNO055 Absolute
 *   Orientation Sensor over I2C on a QNX system. It:
 * - Initializes the sensor and verifies its chip ID (expected value: 0xA0).
 * - Implements a calibration system:
 * - Attempts to load existing calibration data from a file.
 * - If no file exists or if `--calibrate` is passed as an argument, it
 *   initiates a guided, manual calibration process.
 * - Saves the new calibration data to a file for future use.
 * - Sets the sensor to NDOF (Nine Degrees of Freedom) fusion mode.
 * - Continuously reads and prints all fused and raw sensor data, including:
 * - Euler Angles (Heading, Roll, Pitch)
 * - Quaternions
 * - Accelerometer (raw, linear, and gravity vector)
 * - Gyroscope
 * - Magnetometer
 *
 * Wiring: (Raspberry Pi header mapping):
 * - VIN: Connect to PIN 1 (3.3V Power).
 * - GND: Connect to PIN 6 (Ground).
 * - SCL: Connect to PIN 5 (GPIO 3).
 * - SDA: Connect to PIN 3 (GPIO 2).
 * - RST: Connect to PIN 13 (GPIO 27) - Note: Reset logic is not used in this example.
 * - INT: Connect to PIN 15 (GPIO 22) - Note: Interrupts are not used in this example.
 *
 * Data Scaling:
 * - Accelerometer, Linear Acceleration, Gravity: 1m/s² = 100 LSB
 * - Magnetometer: 1uT = 16 LSB
 * - Gyroscope: 1°/s = 16 LSB
 * - Euler Angles: 1° = 16 LSB
 * - Quaternion: 1 LSB = 2^-14
 *
 * - Compile:
 *   qcc -Vgcc_ntoaarch64le -o imu-bno055 imu-bno055.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "rpi_i2c.h" 
 
/* --------------------------------------------------------------------------
 * BNO055 Configuration and Definitions
 * ------------------------------------------------------------------------*/
 
// --- I2C Configuration ---
#define BNO055_ADDRESS_A        (0x28)             ///< Default I2C address for the BNO055
#define BNO055_I2C_BUS          1                  ///< I2C bus number
#define CALIBRATION_FILENAME    "bno055_calib.dat" ///< File to store calibration data
 
// --- BNO055 Register Map ---
#define BNO055_CHIP_ID_ADDR          (0x00)  ///< Chip ID Register; expected value: 0xA0
#define BNO055_OPR_MODE_ADDR         (0x3D)  ///< Operation Mode Register
#define BNO055_SYS_TRIGGER_ADDR      (0x3F)  ///< System Trigger Register (for reset)
#define BNO055_CALIB_STAT_ADDR       (0x35)  ///< Calibration Status Register
#define BNO055_ACC_OFFSET_X_LSB_ADDR (0x55)  ///< Start of calibration data registers
#define CALIBRATION_DATA_SIZE        (22)    ///< Size of the calibration data block (22 bytes)
 
// --- Data Registers ---
#define BNO055_ACC_DATA_X_LSB_ADDR  (0x08)  ///< Accelerometer data start
#define BNO055_MAG_DATA_X_LSB_ADDR  (0x0E)  ///< Magnetometer data start
#define BNO055_GYR_DATA_X_LSB_ADDR  (0x14)  ///< Gyroscope data start
#define BNO055_EUL_HEADING_LSB_ADDR (0x1A)  ///< Euler angle data start
#define BNO055_QUA_DATA_W_LSB_ADDR  (0x20)  ///< Quaternion data start
#define BNO055_LIA_DATA_X_LSB_ADDR  (0x28)  ///< Linear Acceleration data start
#define BNO055_GRV_DATA_X_LSB_ADDR  (0x2E)  ///< Gravity Vector data start
 
// --- BNO055 Operation Modes ---
#define BNO055_OPERATION_MODE_CONFIG (0x00) ///< Configuration mode (for setup)
#define BNO055_OPERATION_MODE_NDOF   (0x0C) ///< 9-DOF absolute orientation fusion mode

// --- Data Scaling Divisors (LSB to physical units) ---
#define BNO055_ACCEL_DIVISOR    100.0  // 1m/s² = 100 LSB
#define BNO055_MAG_DIVISOR      16.0   // 1uT = 16 LSB
#define BNO055_GYRO_DIVISOR     16.0   // 1dps = 16 LSB
#define BNO055_EULER_DIVISOR    16.0   // 1° = 16 LSB
#define BNO055_QUAT_SCALE       (1.0 / (1 << 14)) // Scale for quaternion values
 
/* --------------------------------------------------------------------------
 * Data Structures
 * ------------------------------------------------------------------------*/
 
/**
 * @brief Stores a 3-axis vector (e.g., for accelerometer, gyroscope).
 */
typedef struct {
    double x, y, z;
} bno055_vector_t;
 
/**
 * @brief Stores a 4-component quaternion.
 */
typedef struct {
    double w, x, y, z;
} bno055_quaternion_t;

/* --------------------------------------------------------------------------
* Function Prototypes
* ------------------------------------------------------------------------*/
 
int bno055_set_mode(uint8_t mode);
int bno055_save_calibration(const char* filename);
int bno055_load_calibration(const char* filename);
void bno055_perform_manual_calibration();
void read_all_sensor_data();
int bno055_read_vector(uint8_t reg, bno055_vector_t *vec, double divisor);
int bno055_read_quaternion(bno055_quaternion_t *quat);

/* --------------------------------------------------------------------------
 * Function Implementations
 * ------------------------------------------------------------------------*/
 
/**
 * @brief Reads all available sensor data and prints it to the console.
 */
void read_all_sensor_data() {
    bno055_vector_t euler, accel, mag, gyro, lia, grv;
    bno055_quaternion_t quat;
 
    // Read all sensor vectors using the defined constants
    bno055_read_vector(BNO055_EUL_HEADING_LSB_ADDR, &euler, BNO055_EULER_DIVISOR);
    bno055_read_vector(BNO055_ACC_DATA_X_LSB_ADDR, &accel, BNO055_ACCEL_DIVISOR);
    bno055_read_vector(BNO055_MAG_DATA_X_LSB_ADDR, &mag, BNO055_MAG_DIVISOR);
    bno055_read_vector(BNO055_GYR_DATA_X_LSB_ADDR, &gyro, BNO055_GYRO_DIVISOR);
    bno055_read_vector(BNO055_LIA_DATA_X_LSB_ADDR, &lia, BNO055_ACCEL_DIVISOR); // Linear Accel shares divisor with Accel
    bno055_read_vector(BNO055_GRV_DATA_X_LSB_ADDR, &grv, BNO055_ACCEL_DIVISOR); // Gravity shares divisor with Accel
    bno055_read_quaternion(&quat);
 
    printf("--------------------------------------------------\n");
    printf("Euler (H,R,P):        %7.2f, %7.2f, %7.2f deg\n", euler.x, euler.y, euler.z);
    printf("Quaternion (W,X,Y,Z): %7.2f, %7.2f, %7.2f, %7.2f\n", quat.w, quat.x, quat.y, quat.z);
    printf("Accelerometer (m/s^2):  X=%7.2f, Y=%7.2f, Z=%7.2f\n", accel.x, accel.y, accel.z);
    printf("Linear Accel (m/s^2):   X=%7.2f, Y=%7.2f, Z=%7.2f\n", lia.x, lia.y, lia.z);
    printf("Gravity Vector (m/s^2): X=%7.2f, Y=%7.2f, Z=%7.2f\n", grv.x, grv.y, grv.z);
    printf("Gyroscope (dps):        X=%7.2f, Y=%7.2f, Z=%7.2f\n", gyro.x, gyro.y, gyro.z);
    printf("Magnetometer (uT):      X=%7.2f, Y=%7.2f, Z=%7.2f\n", mag.x, mag.y, mag.z);
}
 
/**
 * @brief Reads a 3-axis vector from the BNO055.
 *
 * @param reg The starting register address for the 3-axis data (6 bytes).
 * @param vec Pointer to a `bno055_vector_t` struct to store the result.
 * @param divisor The scaling factor to convert the raw integer value to physical units.
 * @return int 0 on success, -1 on I2C read error.
 */
int bno055_read_vector(uint8_t reg, bno055_vector_t *vec, double divisor) {
    uint8_t buffer[6];
    if (smbus_read_block_data(BNO055_I2C_BUS, BNO055_ADDRESS_A, reg, buffer, 6) != 0) {
        return -1;
    }
 
    int16_t x = (int16_t)(((uint16_t)buffer[1] << 8) | (uint16_t)buffer[0]);
    int16_t y = (int16_t)(((uint16_t)buffer[3] << 8) | (uint16_t)buffer[2]);
    int16_t z = (int16_t)(((uint16_t)buffer[5] << 8) | (uint16_t)buffer[4]);
 
    vec->x = (double)x / divisor;
    vec->y = (double)y / divisor;
    vec->z = (double)z / divisor;
 
    return 0;
}
 
/**
 * @brief Reads the quaternion data from the BNO055.
 *
 * @param quat Pointer to a `bno055_quaternion_t` struct to store the result.
 * @return int 0 on success, -1 on I2C read error.
 */
int bno055_read_quaternion(bno055_quaternion_t *quat) {
    uint8_t buffer[8];
    if (smbus_read_block_data(BNO055_I2C_BUS, BNO055_ADDRESS_A, BNO055_QUA_DATA_W_LSB_ADDR, buffer, 8) != 0) {
        return -1;
    }
 
    double scale = (1.0 / (1 << 14)); // 1.0 / 16384.0
 
    quat->w = (double)((int16_t)(((uint16_t)buffer[1] << 8) | (uint16_t)buffer[0])) * scale;
    quat->x = (double)((int16_t)(((uint16_t)buffer[3] << 8) | (uint16_t)buffer[2])) * scale;
    quat->y = (double)((int16_t)(((uint16_t)buffer[5] << 8) | (uint16_t)buffer[4])) * scale;
    quat->z = (double)((int16_t)(((uint16_t)buffer[7] << 8) | (uint16_t)buffer[6])) * scale;
 
    return 0;
}
 
/**
 * @brief Sets the operation mode of the BNO055.
 *
 * @param mode The desired operation mode (e.g., BNO055_OPERATION_MODE_CONFIG).
 * @return int 0 on success, -1 on I2C write error.
 */
int bno055_set_mode(uint8_t mode) {
    if (smbus_write_byte_data(BNO055_I2C_BUS, BNO055_ADDRESS_A, BNO055_OPR_MODE_ADDR, mode) != 0) {
        perror("Failed to set operation mode");
        return -1;
    }
    return 0;
}
 
/**
 * @brief Guides the user through a manual calibration process.
 *
 * This function continuously polls the calibration status register and prints
 * the status for the system, gyroscope, accelerometer, and magnetometer.
 * It waits until the gyroscope, accelerometer, and magnetometer all report
 * full calibration (status 3) before completing.
 */
void bno055_perform_manual_calibration() {
    uint8_t calib_stat = 0;
    printf("\n--- Starting Manual Calibration ---\n");
    printf("Please perform the calibration dance until all components are 3:\n");
    printf("1. Gyroscope (GYR): Place on a flat, stable surface.\n");
    printf("2. Accelerometer (ACC): Slowly move to ~6 different stable orientations.\n");
    printf("3. Magnetometer (MAG): Slowly make a large figure-8 motion.\n\n");
 
    while (1) {
        smbus_read_byte_data(BNO055_I2C_BUS, BNO055_ADDRESS_A, BNO055_CALIB_STAT_ADDR, &calib_stat);
 
        uint8_t sys_calib = (calib_stat >> 6) & 0x03;
        uint8_t gyro_calib = (calib_stat >> 4) & 0x03;
        uint8_t accel_calib = (calib_stat >> 2) & 0x03;
        uint8_t mag_calib = calib_stat & 0x03;
 
        printf("Calibration Status: SYS=%d, GYR=%d, ACC=%d, MAG=%d\r", sys_calib, gyro_calib, accel_calib, mag_calib);
        fflush(stdout);
 
        // Wait until individual sensors are fully calibrated.
        if (gyro_calib == 3 && accel_calib == 3 && mag_calib == 3) {
            // Once individual sensors are good, the system fusion (SYS) will also stabilize.
            if (sys_calib == 3) {
                 printf("\nFull System Calibration Complete!\n");
                 break;
            }
        }
        usleep(100000);
    }
}
 
/**
 * @brief Saves the sensor's calibration data to a file.
 *
 * This function switches the sensor to CONFIG mode, reads the 22-byte
 * calibration offset and radius data, and writes it to the specified file.
 *
 * @param filename The name of the file to save the data to.
 * @return int 0 on success, -1 on failure.
 */
int bno055_save_calibration(const char* filename) {
    uint8_t calib_data[CALIBRATION_DATA_SIZE];
 
    // Switch to CONFIG mode to read calibration data
    bno055_set_mode(BNO055_OPERATION_MODE_CONFIG);
    usleep(25000);
 
    if (smbus_read_block_data(BNO055_I2C_BUS, BNO055_ADDRESS_A, BNO055_ACC_OFFSET_X_LSB_ADDR, calib_data, CALIBRATION_DATA_SIZE) != 0) {
        perror("Failed to read calibration data from sensor");
        return -1;
    }
 
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open calibration file for writing");
        return -1;
    }
 
    fwrite(calib_data, 1, CALIBRATION_DATA_SIZE, fp);
    fclose(fp);
    printf("Successfully saved calibration data to %s\n", filename);
    return 0;
}
 
/**
 * @brief Loads calibration data from a file and writes it to the sensor.
 *
 * This function reads the 22-byte calibration data from the specified file,
 * switches the sensor to CONFIG mode, and writes the data to the sensor's
 * offset and radius registers.
 *
 * @param filename The name of the file to load data from.
 * @return int 0 on success, -1 on failure (e.g., file not found).
 */
int bno055_load_calibration(const char* filename) {
    uint8_t calib_data[CALIBRATION_DATA_SIZE];
 
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("INFO: Calibration file not found. Starting manual calibration.\n");
        return -1;
    }
 
    fread(calib_data, 1, CALIBRATION_DATA_SIZE, fp);
    fclose(fp);
 
    // Switch to CONFIG mode to write calibration data
    bno055_set_mode(BNO055_OPERATION_MODE_CONFIG);
    usleep(25000);
 
    if (smbus_write_block_data(BNO055_I2C_BUS, BNO055_ADDRESS_A, BNO055_ACC_OFFSET_X_LSB_ADDR, calib_data, CALIBRATION_DATA_SIZE) != 0) {
        perror("Failed to write calibration data to sensor");
        return -1;
    }
    printf("Successfully loaded calibration data from %s\n", filename);
    return 0;
}

/* --------------------------------------------------------------------------
 * Main Application
 * ------------------------------------------------------------------------*/
 
/**
 * @brief Main function.
 *
 * Initializes the BNO055, handles calibration loading or execution,
 * and enters a loop to continuously read and display sensor data.
 *
 * @param argc Argument count.
 * @param argv Argument vector. Can accept "--calibrate" to force recalibration.
 * @return int EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int main(int argc, char *argv[]) {
    int boot_mode = 0; // 0 for Normal, 1 for Forced Calibration
 
    if (argc > 1 && strcmp(argv[1], "--calibrate") == 0) {
        boot_mode = 1;
        printf("BOOT MODE: Forced Calibration\n");
    } else {
        printf("BOOT MODE: Normal\n");
    }
 
    // Verify sensor presence
    uint8_t chip_id = 0;
    if (smbus_read_byte_data(BNO055_I2C_BUS, BNO055_ADDRESS_A, BNO055_CHIP_ID_ADDR, &chip_id) != 0 || chip_id != 0xA0) {
        perror("BNO055 not found at I2C address 0x28. Check wiring.");
        return EXIT_FAILURE;
    }
    printf("BNO055 Chip ID: 0x%02X\n", chip_id);
 
    // Reset the sensor
    bno055_set_mode(BNO055_OPERATION_MODE_CONFIG);
    smbus_write_byte_data(BNO055_I2C_BUS, BNO055_ADDRESS_A, BNO055_SYS_TRIGGER_ADDR, 0x20);
    usleep(700000); // Wait for reset to complete
 
    // Load calibration or perform manual calibration
    if (boot_mode == 1 || bno055_load_calibration(CALIBRATION_FILENAME) != 0) {
        bno055_set_mode(BNO055_OPERATION_MODE_NDOF);
        usleep(25000);
        bno055_perform_manual_calibration();
        bno055_save_calibration(CALIBRATION_FILENAME);
    }
 
    // Set final operation mode for data reading
    printf("Setting final operation mode to NDOF...\n");
    bno055_set_mode(BNO055_OPERATION_MODE_NDOF);
    usleep(25000);
 
    // Main loop
    printf("\n--- Starting Continuous Data Readout ---\n");
    while(1) {
        read_all_sensor_data();
        usleep(100000); // 100ms delay, ~10Hz
    }
 
    return EXIT_SUCCESS;
}
 