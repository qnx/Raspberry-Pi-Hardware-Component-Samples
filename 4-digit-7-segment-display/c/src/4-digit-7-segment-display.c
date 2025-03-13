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

#include "rpi_gpio.h" // Raspberry Pi GPIO control library

// Define GPIO pins for each segment of the 7-segment display
#define SEG_A GPIO23
#define SEG_B GPIO6
#define SEG_C GPIO20
#define SEG_D GPIO5
#define SEG_E GPIO24
#define SEG_F GPIO19
#define SEG_G GPIO12
#define SEG_DP GPIO21 // Decimal point segment

// Define GPIO pins for digit control (for a 4-digit display)
#define DIGIT_1 GPIO18
#define DIGIT_2 GPIO13
#define DIGIT_3 GPIO26
#define DIGIT_4 GPIO25

// Representation of digits (0-9) on the 7-segment display
// Each row corresponds to a digit, and each column represents a segment (A-G, DP)
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
    {1, 1, 1, 1, 0, 1, 1, 0}   // 9
};

// Array of digit control pins (common anode display)
int digit_pins[4] = {DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4};

// Initialize a given GPIO pin as an output
int init_gpio(int gpio_pin) {
    if (rpi_gpio_setup(gpio_pin, GPIO_OUT)) {
        perror("rpi_gpio_setup");
        return -1;
    }
    return 0;
}

// Turn off all digit control lines (disable all digits)
void clear_digits() {
    for (int i = 0; i < 4; i++) {
        rpi_gpio_output(digit_pins[i], GPIO_HIGH);  // Common anode: HIGH = OFF
    }
}

// Display a single digit on a specified position
void display_digit(int digit, int position) {
    clear_digits(); // Turn off all digits before enabling a new one
    
    // Get the segment pattern for the given digit
    int *pattern = digits[digit];
    
    // Set the segment states based on the pattern
    rpi_gpio_output(SEG_A, pattern[0] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_B, pattern[1] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_C, pattern[2] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_D, pattern[3] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_E, pattern[4] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_F, pattern[5] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_G, pattern[6] ? GPIO_HIGH : GPIO_LOW);
    rpi_gpio_output(SEG_DP, pattern[7] ? GPIO_HIGH : GPIO_LOW);
    
    // Enable the specific digit position (LOW = ON for common anode)
    rpi_gpio_output(digit_pins[position], GPIO_LOW);
    
    usleep(1000); // Small delay to allow persistence of display
}

// Display a 4-digit number on the 7-segment display
void display_number(int num) {
    char num_str[5];
    snprintf(num_str, sizeof(num_str), "%04d", num); // Format number as 4-digit string
    
    for (int i = 0; i < 4; i++) {
        int digit = num_str[i] - '0'; // Convert character to integer
        display_digit(digit, i);
    }
}

int main() {
    // Initialize all GPIO pins used for the segments and digit control
    int all_pins[] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_DP, 
                       DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4};
    
    for (int i = 0; i < 12; i++) {
        if (init_gpio(all_pins[i]) == -1) {
            return EXIT_FAILURE; // Exit if any GPIO setup fails
        }
    }
    
    // Continuously display numbers from 0000 to 9999
    while (1) {
        for (int num = 0; num < 10000; num++) { // Loop through numbers 0-9999
            for (int i = 0; i < 10; i++) { // Multiplex each number for stability
                display_number(num);
            }
        }
    }
    
    return EXIT_SUCCESS;
}