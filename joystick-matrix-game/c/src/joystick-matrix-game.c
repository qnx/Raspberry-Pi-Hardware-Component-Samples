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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "rpi_spi.h"
 
// ********************************************************************************************
// Joystick related functions
// ********************************************************************************************
 
// SPI settings for joystick (MCP3008 ADC)
#define JOYSTICK_BUS 0 // Matrix is also on BUS 0
#define JOYSTICK_DEVICE 0 // Joystick is device 0, matrix is device 1
#define JOYSTICK_SPI_SPEED 1000000 // 1 MHz

// MCP3008 SPI command bits
#define MCP3008_START_BIT      0x01
#define MCP3008_SGL_DIFF       0x08  // Single-ended mode
#define MCP3008_DONT_CARE      0x00

// Joystick calibration
#define JOY_CENTER 512
#define DEADZONE   100

// MCP3008 has 8 input channels
#define MCP3008_MAX_CHANNEL 7

// Enumeration of possible joystick directions
typedef enum {
    DIR_CENTER,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP_LEFT,
    DIR_UP_RIGHT,
    DIR_DOWN_LEFT,
    DIR_DOWN_RIGHT
} JoystickDirection;
 
// Control flag for main loop
bool running = true;

// Reads a 10-bit analog value from the MCP3008 on the specified channel
int mcp3008_read_channel(uint8_t channel, uint16_t *result) {
    if (channel > MCP3008_MAX_CHANNEL || result == NULL) {
        return 0;
    }

    uint8_t write_buffer[3];
    write_buffer[0] = MCP3008_START_BIT;
    write_buffer[1] = (MCP3008_SGL_DIFF | channel) << 4;
    write_buffer[2] = MCP3008_DONT_CARE;

    uint8_t read_buffer[3] = {0};

    if (rpi_spi_write_read_data(JOYSTICK_BUS, JOYSTICK_DEVICE, write_buffer, read_buffer, 3) != SPI_SUCCESS) {
        fprintf(stderr, "SPI read failed\n");
        return 0;
    }

    *result = ((read_buffer[1] & 0x03) << 8) | read_buffer[2];  // Combine 10-bit result
    return 1;
}

// Converts raw x and y analog values to a joystick direction
JoystickDirection get_joystick_direction(int x, int y) {
    int dx = x - JOY_CENTER;
    int dy = y - JOY_CENTER;

    int is_left   = dx < -DEADZONE;
    int is_right  = dx >  DEADZONE;
    int is_up     = dy < -DEADZONE;
    int is_down   = dy >  DEADZONE;

    if (is_up && is_left) return DIR_UP_LEFT;
    if (is_up && is_right) return DIR_UP_RIGHT;
    if (is_down && is_left) return DIR_DOWN_LEFT;
    if (is_down && is_right) return DIR_DOWN_RIGHT;
    if (is_up) return DIR_UP;
    if (is_down) return DIR_DOWN;
    if (is_left) return DIR_LEFT;
    if (is_right) return DIR_RIGHT;

    return DIR_CENTER;
}

// Converts joystick direction enum to a human-readable string
const char* joystick_direction_to_string(JoystickDirection dir) {
    switch (dir) {
        case DIR_CENTER:     return "CENTER";
        case DIR_UP:         return "UP";
        case DIR_DOWN:       return "DOWN";
        case DIR_LEFT:       return "LEFT";
        case DIR_RIGHT:      return "RIGHT";
        case DIR_UP_LEFT:    return "UP_LEFT";
        case DIR_UP_RIGHT:   return "UP_RIGHT";
        case DIR_DOWN_LEFT:  return "DOWN_LEFT";
        case DIR_DOWN_RIGHT: return "DOWN_RIGHT";
        default:             return "UNKNOWN";
    }
}

// ********************************************************************************************
// Matrix related functions
// ********************************************************************************************

// SPI settings for LED matrix (MAX7219)
#define MATRIX_BUS 0 // Joystick is also on BUS 0
#define MATRIX_DEVICE 1 // Joystick is device 0, matrix is device 1
#define MATRIX_SPI_SPEED 1000000 // 1 MHz

// MAX7219 command registers
#define CMD_DIGIT0     0x01
#define CMD_DECODEMODE 0x09
#define CMD_BRIGHTNESS 0x0A
#define CMD_SCANLIMIT  0x0B
#define CMD_SHUTDOWN   0x0C
#define CMD_DISPLAYTEST 0x0F

// Current state of each row on the LED matrix
uint8_t matrixState[8] = {0};

// The players position in the matrix
int matrixX = 0;
int matrixY = 0;

// Sends a register/value command to the MAX7219
int max7219_send(uint8_t reg, uint8_t val) {
    uint8_t buffer[2] = {reg, val};
    if (rpi_spi_write_read_data(MATRIX_BUS, MATRIX_DEVICE, buffer, NULL, 2) != SPI_SUCCESS) {
        fprintf(stderr, "Failed to send SPI data\n");
        return 0;
    }
    return 1;
}

// Clears all LEDs on the matrix
int max7219_clear() {
    for (uint8_t i = 0; i < 8; i++) {
        if (!max7219_send(CMD_DIGIT0 + i, 0x00)) {
            fprintf(stderr, "Failed to clear display\n");
            return 0;
        }
    }
    return 1;
}
 
// Initializes the MAX7219 for use with an 8x8 LED matrix
int max7219_init(uint8_t intensity) {
    if (rpi_spi_configure_device(MATRIX_BUS, MATRIX_DEVICE,
        SPI_MODE_BODER_MSB | SPI_MODE_WORD_WIDTH_8 | SPI_MODE_CPHA_0,
        MATRIX_SPI_SPEED) != SPI_SUCCESS) {
        fprintf(stderr, "Failed to configure SPI device\n");
        return 0;
    }

    // Initialization sequence
    if (!max7219_send(CMD_SHUTDOWN, 0) ||            // Enter shutdown
        !max7219_send(CMD_DISPLAYTEST, 0) ||         // Disable test mode
        !max7219_send(CMD_SCANLIMIT, 7) ||           // Enable all rows
        !max7219_send(CMD_DECODEMODE, 0) ||          // Use raw LED values
        !max7219_send(CMD_BRIGHTNESS, intensity) ||  // Set brightness
        !max7219_send(CMD_SHUTDOWN, 1)) {            // Turn on display

        fprintf(stderr, "Failed to send init command to MAX7219\n");
        return 0;
    }

    return max7219_clear(); // Clear the screen on startup
}
 
// Lights up an individual pixel at (x, y)
void lightPixel(uint8_t x, uint8_t y) {
    if (x > 7 || y > 7) return;
    matrixState[y] |= (1 << (7 - x));
    max7219_send(CMD_DIGIT0 + y, matrixState[y]);
}

// Turns off an individual pixel at (x, y)
void clearPixel(uint8_t x, uint8_t y) {
    if (x > 7 || y > 7) return;
    matrixState[y] &= ~(1 << (7 - x));
    max7219_send(CMD_DIGIT0 + y, matrixState[y]);
}

// Shows a flashing "win" animation (X across the matrix)
void showWinMatrix() {
    max7219_clear();
    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 8; i++) {
            lightPixel(i, i);        // Diagonal from top-left to bottom-right
            lightPixel(7 - i, i);    // Diagonal from top-right to bottom-left
        }
        usleep(100000);  // Pause
        max7219_clear(); // Flash effect
        usleep(50000);
    }
}

// ********************************************************************************************
// Game related functions
// ********************************************************************************************

// Food-related game mechanics
#define FOOD_SPAWN_INTERVAL_NS 1000000000L // 1 second

// Food grid and tracking
static int foodMatrix[8][8] = {{0}};
static int foodRemaining = 0;
static struct timespec lastFoodSpawn = {0};

// Updates player position based on direction input
void updatePosition(int *x, int *y, JoystickDirection dir) {
    switch (dir) {
        case DIR_UP:         (*y)--; break;
        case DIR_DOWN:       (*y)++; break;
        case DIR_LEFT:       (*x)--; break;
        case DIR_RIGHT:      (*x)++; break;
        case DIR_UP_LEFT:    (*x)--; (*y)--; break;
        case DIR_UP_RIGHT:   (*x)++; (*y)--; break;
        case DIR_DOWN_LEFT:  (*x)--; (*y)++; break;
        case DIR_DOWN_RIGHT: (*x)++; (*y)++; break;
        default: break;
    }

    // Clamp to 0–7 bounds
    if (*x < 0) *x = 0;
    if (*x > 7) *x = 7;
    if (*y < 0) *y = 0;
    if (*y > 7) *y = 7;
}

// Draw the food as pixels on the matrix
void drawFood() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (matrixState[j] & (1 << (7 - i))) {
                continue; // Skip if player is occupying this pixel
            }

            if (foodMatrix[j][i]) {
                lightPixel(i, j);
            }
        }
    }
}

// Spawns a new food item at a random empty location
void spawnFood() {
    int x, y;
    do {
        x = rand() % 8;
        y = rand() % 8;
    } while (foodMatrix[y][x] || (matrixX == x && matrixY == y));

    foodMatrix[y][x] = 1;
    foodRemaining++;
}

// Checks if enough time has passed to spawn new food, and if so spawns a single food
void maybeSpawnFood() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    long elapsed_ns = (now.tv_sec - lastFoodSpawn.tv_sec) * 1000000000L + (now.tv_nsec - lastFoodSpawn.tv_nsec);

    if (elapsed_ns >= FOOD_SPAWN_INTERVAL_NS) {
        spawnFood();
        lastFoodSpawn = now;
    }
}

// Checks if the player has moved onto a food item
void checkFoodCollision() {
    if (foodMatrix[matrixY][matrixX]) {
        foodMatrix[matrixY][matrixX] = 0;
        foodRemaining--;
    }
}

// ********************************************************************************************
// Main program loop
// ********************************************************************************************

int main() {
    srand(time(NULL)); // Seed RNG

    // Init SPI joystick
    if (rpi_spi_configure_device(JOYSTICK_BUS, JOYSTICK_DEVICE,
        SPI_MODE_BODER_MSB | SPI_MODE_WORD_WIDTH_8 | SPI_MODE_CPHA_0,
        JOYSTICK_SPI_SPEED) != SPI_SUCCESS) {
        fprintf(stderr, "Failed to configure joystick SPI\n");
        return 1;
    }

    // Init LED matrix
    if (!max7219_init(0x08)) {
        fprintf(stderr, "Failed to initialize LED matrix\n");
        return 1;
    }

    // Game loop
    while (running) {
        uint16_t xVal, yVal;

        // Read joystick position
        if (!mcp3008_read_channel(0, &xVal) || !mcp3008_read_channel(1, &yVal)) {
            fprintf(stderr, "Failed to read joystick\n");
            break;
        }

        JoystickDirection dir = get_joystick_direction(xVal, yVal);

        // Clear player pixel from old position
        clearPixel(matrixX, matrixY);

        // Update player position based on joystick direction
        updatePosition(&matrixX, &matrixY, dir);

        // Check for collision with food
        checkFoodCollision();

        // Draw food on matrix
        drawFood();

        // Draw new player position
        lightPixel(matrixX, matrixY);

        // Spawn food if needed
        maybeSpawnFood();

        // Win condition: all food collected
        if (foodRemaining == 0) {
            showWinMatrix();
            foodRemaining = 0;
            memset(foodMatrix, 0, sizeof(foodMatrix));
            running = false;
        }

        usleep(100000); // Delay for stability (100ms)
    }

    return 0;
}