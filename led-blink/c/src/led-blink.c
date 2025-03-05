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
#include <time.h>

// The interface for a GPIO resource manager tailored for the Raspberry Pi's GPIO pins under QNX.
// This file provides functions to configure GPIO pins, set pin modes, and read/write pin values.
#include "rpi_gpio.h"

// Define the GPIO pin number (pin 36 on Raspberry Pi 4, which corresponds to GPIO16)
#define GPIO_PIN GPIO16

// Initializes the specified GPIO pin for controlling an LED by setting it as an output pin.
static bool init_led(int gpio_pin)
{
    // Configure the given GPIO pin as an output pin.
    if (rpi_gpio_setup(gpio_pin, GPIO_OUT))
    {
        perror("rpi_gpio_setup");
        return false;
    }

    return true;
}

// Turns on the LED connected to the specified GPIO pin.
static bool led_on(int gpio_pin)
{
    // Set selected GPIO to high.
    if (rpi_gpio_output(gpio_pin, GPIO_HIGH))
    {
        perror("rpi_gpio_output");
        return false;
    }

    return true;
}

// Turns off the LED connected to the specified GPIO pin.
static bool led_off(int gpio_pin)
{
    // Set selected GPIO to low.
    if (rpi_gpio_output(gpio_pin, GPIO_LOW))
    {
        perror("rpi_gpio_output");
        return false;
    }

    return true;
}

int main(void) {

    if (!init_led(GPIO_PIN))
    {
        return EXIT_FAILURE; // Exit the program with a failure status
    }

    // Infinite loop to blink the LED
    while (1) {

        // Turn the LED off
        if (!led_off(GPIO_PIN))
        {
            return EXIT_FAILURE;
        }

        // Leave the LED off for .5 seconds
        delay(500);

        // Turn the LED on
        if (!led_on(GPIO_PIN))
        {
            return EXIT_FAILURE;
        }

        // Leave the LED on for .5 seconds
        delay(500);
    }

    // Exit successfully
    return EXIT_SUCCESS;
}