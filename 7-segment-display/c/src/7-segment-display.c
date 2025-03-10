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
#include <unistd.h>  // For usleep

// The interface for a GPIO resource manager tailored for the Raspberry Pi's GPIO pins under QNX.
#include "rpi_gpio.h"

// Define GPIO pins for each segment of the 7-segment display
#define SEG_A GPIO19
#define SEG_B GPIO26
#define SEG_C GPIO25
#define SEG_D GPIO20
#define SEG_E GPIO21
#define SEG_F GPIO13
#define SEG_G GPIO6
#define SEG_DP GPIO12

// Segment configurations for digits 0-9 (Common Cathode display)
// 1 = ON, 0 = OFF (Each list represents the segments that should be lit for a given number)
int digits[10][8] = {
    {1, 1, 1, 1, 1, 1, 0, 0},  // 0
    {0, 1, 1, 0, 0, 0, 0, 0},  // 1
    {1, 1, 0, 1, 1, 0, 1, 0},  // 2
    {1, 1, 1, 1, 0, 0, 1, 0},  // 3
    {0, 1, 1, 0, 0, 1, 1, 0},  // 4
    {1, 0, 1, 1, 0, 1, 1, 0},  // 5
    {1, 0, 1, 1, 1, 1, 1, 0},  // 6
    {1, 1, 1, 0, 0, 0, 0, 0},  // 7
    {1, 1, 1, 1, 1, 1, 1, 0},  // 8
    {1, 1, 1, 1, 0, 1, 1, 1}   // 9 and decimal point
};

// Initializes a GPIO pin as an output pin
int init_gpio(int gpio_pin) {
    if (rpi_gpio_setup(gpio_pin, GPIO_OUT)) {
        perror("rpi_gpio_setup");
        return -1;
    }
    return 0;
}

// Set the segments according to the digit to display
int display_number(int num) {
    if (num < 0 || num > 9) {
        return -1;  // Invalid number
    }

    // Set the corresponding segments for the number
    int* pattern = digits[num];

    rpi_gpio_output(SEG_A, pattern[0] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_B, pattern[1] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_C, pattern[2] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_D, pattern[3] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_E, pattern[4] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_F, pattern[5] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_G, pattern[6] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_DP, pattern[7] ? GPIO_HIGH : GPIO_LOW);

    return 0;
}

int main() {

    // Initialize all GPIO pins for the 7-segment display
    if (init_gpio(SEG_A) == -1 || init_gpio(SEG_B) == -1 || init_gpio(SEG_C) == -1 ||
        init_gpio(SEG_D) == -1 || init_gpio(SEG_E) == -1 || init_gpio(SEG_F) == -1 ||
        init_gpio(SEG_G) == -1 || init_gpio(SEG_DP) == -1) {
        return EXIT_FAILURE;
    }

    // Loop through digits 0-9
    while (1) {
        for (int i = 0; i < 10; i++) {
            // Display the current digit
            if (display_number(i) == -1) {
                return EXIT_FAILURE;
            }

            // Keep the digit displayed for 1 second
            usleep(1000000);  // 1 second delay
        }
    }

    return EXIT_SUCCESS;
}
