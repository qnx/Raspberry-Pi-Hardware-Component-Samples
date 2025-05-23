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

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>     // Needed for `bool`
#include <signal.h>      // Needed for signal handling
#include "rpi_spi.h"     // SPI interface header

// SPI bus and device configuration
#define BUS 0
#define DEVICE 0
#define SPI_SPEED 1000000 // 1 MHz is sufficient for MCP3008

// MCP3008 protocol constants
#define MCP3008_START_BIT      0x01
#define MCP3008_SGL_DIFF       0x08  // Single-ended mode
#define MCP3008_DONT_CARE      0x00

// Flag to control main loop execution
bool running = true;

// Read 10-bit analog data from the specified MCP3008 channel
int mcp3008_read_channel(uint8_t channel, uint16_t *result) {
    if (channel > 7 || result == NULL) return 0;

    uint8_t write_buffer[3];

    // Prepare SPI message: start bit, single-ended + channel, and dummy byte
    write_buffer[0] = MCP3008_START_BIT;                    // Start bit
    write_buffer[1] = (MCP3008_SGL_DIFF | channel) << 4;    // Channel in bits 6-4
    write_buffer[2] = MCP3008_DONT_CARE;                    // Don't care byte

    uint8_t read_buffer[3] = {0};

    // Perform SPI transaction
    if (rpi_spi_write_read_data(BUS, DEVICE, write_buffer, read_buffer, 3) != SPI_SUCCESS) {
        fprintf(stderr, "SPI read failed\n");
        return 0;
    }

    // Extract 10-bit result from returned bytes
    *result = ((read_buffer[1] & 0x03) << 8) | read_buffer[2];
    return 1;
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

    // Configure SPI interface for MCP3008
    if (rpi_spi_configure_device(BUS, DEVICE,
        SPI_MODE_BODER_MSB | SPI_MODE_WORD_WIDTH_8 | SPI_MODE_CPHA_0,
        SPI_SPEED) != SPI_SUCCESS) {
        fprintf(stderr, "Failed to configure SPI\n");
        return 1;
    }

    // Main loop: read joystick values continuously until stopped
    while (running) {
        uint16_t x = 0, y = 0;

        // Read X and Y channels (0 and 1)
        if (mcp3008_read_channel(0, &x) && mcp3008_read_channel(1, &y)) {
            printf("Joystick X: %4d, Y: %4d\n", x, y);
        } else {
            fprintf(stderr, "Failed to read joystick\n");
        }

        usleep(100000); // Delay 100ms between reads
    }

    // Clean up SPI device before exiting
    rpi_spi_cleanup_device(BUS, DEVICE);

    return 0;
}