/*
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
 */

#include <stdio.h>      // For standard input/output functions
#include <stdint.h>     // For fixed-size integer types like uint8_t, uint16_t
#include <string.h>     // For memory functions if needed
#include <unistd.h>     // For usleep()
#include <rpi_i2c.h>    // I2C abstraction for QNX platform
#include <stdbool.h>    // For using `bool`, `true`, `false`
#include <signal.h>     // For handling termination signals like Ctrl+C

// I2C sensor address and bus configuration
#define SHT3X_ADDR 0x44  // Default I2C address for the SHT3X temperature/humidity sensor
#define BUS 1            // I2C bus number (usually bus 1 on Raspberry Pi)

// I2C commands for SHT3X sensor
const uint8_t SHT3X_BREAK_CMD[]   = {0x30, 0x93}; // Soft reset "break" command
const uint8_t SHT3X_SRESET_CMD[]  = {0x30, 0xA2}; // Software reset command
const uint8_t SHT3X_MEASURE_CMD[] = {0x2C, 0x06}; // High repeatability, no clock stretching

// Struct to hold sensor data
typedef struct {
    float temperature;
    float humidity;
    int valid;  // 1 if data is valid, 0 if error occurred
} SHT3XData;

// Global flag to control the main loop; allows clean exit on signal
bool running = true;

// Initialize the SHT3X sensor by sending soft reset commands
int sht3x_init() {
    // Send "break" command to reset the sensor
    if (smbus_write_block(BUS, SHT3X_ADDR, SHT3X_BREAK_CMD, sizeof(SHT3X_BREAK_CMD)) != I2C_SUCCESS) {
        printf("SHT3X-1: Failed to initialize sensor\n");
        return -1;
    }

    usleep(1000);  // Wait briefly after reset

    // Send software reset command
    if (smbus_write_block(BUS, SHT3X_ADDR, SHT3X_SRESET_CMD, sizeof(SHT3X_SRESET_CMD)) != I2C_SUCCESS) {
        printf("SHT3X-2: Failed to initialize sensor\n");
        return -1;
    }

    usleep(1000);  // Wait briefly again

    return 0;  // Success
}

// Read data from the SHT3X sensor and return it in a SHT3XData struct
SHT3XData sht3x_read() {
    SHT3XData data = {0};         // Initialize all fields to 0
    uint8_t rawData[6] = {0};     // Buffer to hold raw sensor output

    // Request a single-shot measurement
    if (smbus_write_block(BUS, SHT3X_ADDR, SHT3X_MEASURE_CMD, sizeof(SHT3X_MEASURE_CMD)) != I2C_SUCCESS) {
        printf("SHT3X: Failed to send measure command\n");
        data.valid = 0;
        return data;
    }

    usleep(15000);  // Wait 15 milliseconds for measurement to complete

    // Read 6 bytes: temp (2 bytes) + CRC (1), humidity (2 bytes) + CRC (1)
    if (smbus_read_block(BUS, SHT3X_ADDR, rawData, sizeof(rawData)) != I2C_SUCCESS) {
        printf("SHT3X: Failed to read data\n");
        data.valid = 0;
        return data;
    }

    // Convert temperature: bytes 0 and 1
    uint16_t rawTemp = (rawData[0] << 8) | rawData[1];
    float temperature = -45.0f + 175.0f * (float)rawTemp / 65535.0f;

    // Convert humidity: bytes 3 and 4
    uint16_t rawHumidity = (rawData[3] << 8) | rawData[4];
    float humidity = 100.0f * (float)rawHumidity / 65535.0f;

    // Store results
    data.temperature = temperature;
    data.humidity = humidity;
    data.valid = 1;  // Mark data as valid

    return data;
}

// Signal handler to stop the main loop cleanly
static void ctrl_c_handler(int signum) {
    (void)(signum);     // Suppress unused variable warning
    running = false;    // Exit loop on next iteration
}

// Set up signal handling for SIGINT (Ctrl+C) and SIGTERM (kill)
static void setup_handlers(void) {
    struct sigaction sa = {
        .sa_handler = ctrl_c_handler,
    };
    sigaction(SIGINT, &sa, NULL);   // Register handler for Ctrl+C
    sigaction(SIGTERM, &sa, NULL);  // Register handler for termination
}

// Main application entry point
int main() {

    // Initialize the sensor
    if (sht3x_init() != 0) {
        return -1;
    }

    // Register signal handlers
    setup_handlers();

    // Main loop: read and print sensor values until interrupted
    while (running) {
        SHT3XData reading = sht3x_read();

        if (!reading.valid) {
            printf("SHT3X: Failed to get valid reading\n");
        } else {
            printf("Temp: %.2f°C, Humidity: %.2f%%\n", reading.temperature, reading.humidity);
        }

        usleep(500000); // Wait 500 ms before next reading
    }

    // Clean up I2C resources
    smbus_cleanup(BUS);
    return 0;
}
