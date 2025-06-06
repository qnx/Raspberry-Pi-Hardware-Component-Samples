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

#include <stdio.h>      // For printf
#include <stdint.h>     // For uint8_t type
#include <string.h>     // For memory operations (not used here but commonly included)
#include <unistd.h>     // For usleep
#include "rpi_i2c.h"    // Custom I2C API for Raspberry Pi or QNX
#include <stdbool.h>     // Needed for `bool`
#include <signal.h>      // Needed for signal handling

 
#define I2C_ADDR 0x23   // I2C address of the light sensor (e.g., BH1750)
#define BUS 1           // I2C bus number (typically 1 on Raspberry Pi)
 
// Flag to control main loop execution
bool running = true;
 
// Reads 2 bytes of lux data from the sensor and returns the light level in lux
int i2c_read_lux() {
    uint8_t buffer[2];  // Buffer to store the two bytes from the sensor

    // Read two bytes from the sensor over I2C
    int response = smbus_read_block(BUS, I2C_ADDR, buffer, sizeof(buffer));
    if (response != I2C_SUCCESS) {
        printf("Failed to read bytes\n");
        return -1;
    }

    // Combine the two bytes into a 16-bit value
    int lux = (int)(buffer[0]) << 8;  // High byte
    lux |= buffer[1];                 // Low byte

    // Convert raw value to lux using a fixed conversion factor
    return lux / 1.2;
}

// Signal handler to exit the main loop gracefully
static void ctrl_c_handler(int signum) {
    (void)(signum);     // Avoid unused parameter warning
    running = false;
}

// Register signal handlers for SIGINT and SIGTERM
static void setup_handlers(void) {
    struct sigaction sa = {
        .sa_handler = ctrl_c_handler,
    };
    sigaction(SIGINT, &sa, NULL);   // Handle Ctrl+C
    sigaction(SIGTERM, &sa, NULL);  // Handle kill signal
}

int main() {

    // Set up signal handlers
    setup_handlers();

    // Send command to sensor to set it to "continuous high resolution mode"
    int response = smbus_write_byte(BUS, I2C_ADDR, 0x10);
    if (response != I2C_SUCCESS) {
        printf("Failed to write byte\n");
        return -1;
    }

    // Continuously read and print lux values
    while (running) {
        int lux = i2c_read_lux();  // Read light level from sensor

        if (lux == -1) {
            printf("Failed to read lux\n");
            return -1;
        } else {
            printf("lux: %d\n", lux);  // Print lux value to console
        }

        usleep(200000); // Sleep for 200 milliseconds before next read
    }

    // Cleanup I2C resources
    smbus_cleanup(BUS);
    return 0;
}
