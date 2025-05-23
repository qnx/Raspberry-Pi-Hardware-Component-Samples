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
#include "rpi_spi.h"

// SPI configuration constants
#define BUS 0
#define DEVICE 0
#define SPI_SPEED 10000000 // 10 MHz

// MAX7219 register addresses
#define CMD_DIGIT0     0x01 // Base register for digit (row) control
#define CMD_DECODEMODE 0x09 // BCD decode mode
#define CMD_BRIGHTNESS 0x0A // Intensity control
#define CMD_SCANLIMIT  0x0B // Number of digits to scan (0–7)
#define CMD_SHUTDOWN   0x0C // Shutdown control
#define CMD_DISPLAYTEST 0x0F // Display test mode

// Sends a register/value pair to the MAX7219
int max7219_send(uint8_t reg, uint8_t val) {
    uint8_t buffer[2] = {reg, val};
    if (rpi_spi_write_read_data(BUS, DEVICE, buffer, NULL, 2) != SPI_SUCCESS) {
        fprintf(stderr, "Failed to send SPI data\n");
        return 0;
    }
    return 1;
}

// Clears all 8 rows of the 8x8 matrix
int max7219_clear() {
    for (uint8_t i = 0; i < 8; i++) {
        if (!max7219_send(CMD_DIGIT0 + i, 0x00)) {  // Turn off all LEDs in row `i`
            fprintf(stderr, "Failed to clear display\n");
            return 0;
        }
    }
    return 1;
}

// Initializes the MAX7219 chip for an 8x8 LED matrix
// `intensity` sets the brightness (0x00 to 0x0F)
int max7219_init(uint8_t intensity) {

    // Configure SPI device with correct mode and speed
    if (rpi_spi_configure_device(BUS, DEVICE,
        SPI_MODE_BODER_MSB | SPI_MODE_WORD_WIDTH_8 | SPI_MODE_CPHA_0,
        SPI_SPEED) != SPI_SUCCESS) {
        fprintf(stderr, "Failed to configure SPI device\n");
        return 0;
    }

    // Send startup configuration commands to MAX7219
    if (!max7219_send(CMD_SHUTDOWN, 0) ||            // Enter shutdown mode
        !max7219_send(CMD_DISPLAYTEST, 0) ||         // Disable display test
        !max7219_send(CMD_SCANLIMIT, 7) ||           // Enable all 8 digits
        !max7219_send(CMD_DECODEMODE, 0) ||          // No BCD decoding
        !max7219_send(CMD_BRIGHTNESS, intensity) ||  // Set brightness
        !max7219_send(CMD_SHUTDOWN, 1)) {            // Exit shutdown (turn on display)

        fprintf(stderr, "Failed to send init command to MAX7219\n");
        return 0;
    }

    // Clear the display initially
    if (!max7219_clear()) {
        return 0;
    }

    return 1;
}

int main() {

    // Initialize MAX7219 with low brightness
    if (!max7219_init(0x01)) {
        return 1;
    }

    // Light up each row one by one briefly
    for (int i = 0; i < 8; i++) {
        max7219_send(CMD_DIGIT0 + i, 0xFF);  // Turn on all LEDs in row `i`
        usleep(100000);                      // Wait 100ms
        max7219_send(CMD_DIGIT0 + i, 0x00);  // Turn off row `i`
    }

    // Clean up SPI device
    rpi_spi_cleanup_device(BUS, DEVICE);

    return 0;
}
