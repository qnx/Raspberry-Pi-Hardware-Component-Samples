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

#include <stdio.h>    // Standard input/output functions (for puts and perror)
#include <stdlib.h>   // Standard library (for EXIT_SUCCESS and EXIT_FAILURE)
#include <stdbool.h>
#include <unistd.h>  // For usleep function

#include "rpi_gpio.h"  // Raspberry Pi GPIO library

// Define GPIO pins for rows (outputs) and columns (inputs)
#define ROW_PINS  {18, 23, 24, 25}  // Row pins configured as outputs
#define COL_PINS  {12, 16, 20, 21}  // Column pins configured as inputs

// Define the keypad layout mapping row and column indices to characters
const char KEYS[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Function to initialize GPIO pins for keypad scanning
bool init_gpio(int *rows, int *cols) {
    for (int i = 0; i < 4; i++) {
        // Set row pins as outputs and initialize them to HIGH
        if (rpi_gpio_setup(rows[i], GPIO_OUT) || rpi_gpio_output(rows[i], GPIO_HIGH)) {
            perror("Error setting up row pins");
            return false;
        }

        // Set column pins as inputs with pull-up resistors
        if (rpi_gpio_setup_pull(cols[i], GPIO_IN, GPIO_PUD_UP)) {
            perror("Error setting up column pins");
            return false;
        }
    }
    return true;
}

// Function to scan the keypad and return the pressed key
char scan_keypad(int *rows, int *cols) {
    unsigned level;  // Variable to store pin read value

    for (int r = 0; r < 4; r++) {
        // Set the current row LOW to detect key presses
        rpi_gpio_output(rows[r], GPIO_LOW);
        usleep(10000);  // Small delay to allow signal stabilization

        for (int c = 0; c < 4; c++) {
            // Check if the column pin is LOW (key pressed)
            if (rpi_gpio_input(cols[c], &level) == 0 && level == GPIO_LOW) {
                // Wait for key release (debounce)
                while (rpi_gpio_input(cols[c], &level) == 0 && level == GPIO_LOW) {
                    usleep(100000);
                }
                rpi_gpio_output(rows[r], GPIO_HIGH);  // Reset the row pin
                return KEYS[r][c];  // Return the detected key
            }
        }
        rpi_gpio_output(rows[r], GPIO_HIGH);  // Reset the row pin before moving to the next
    }
    return 0;  // No key detected
}

int main() {
    int row_pins[4] = ROW_PINS;  // Assign row pin values
    int col_pins[4] = COL_PINS;  // Assign column pin values

    // Initialize GPIO, exit on failure
    if (!init_gpio(row_pins, col_pins)) {
        return EXIT_FAILURE;
    }

    printf("Keypad ready. Press keys...\n");

    while (1) {
        char key = scan_keypad(row_pins, col_pins);  // Scan for key presses
        if (key) {
            printf("Key Pressed: %c\n", key);  // Print the pressed key
        }
        usleep(200000);  // Delay to avoid excessive scanning
    }

    return EXIT_SUCCESS;
}