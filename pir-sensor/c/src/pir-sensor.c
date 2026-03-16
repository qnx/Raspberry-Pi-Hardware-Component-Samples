/*
 * Copyright (c) 2025-2026, BlackBerry Limited. All rights reserved.
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

#include <fcntl.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>    // Standard input/output functions (for puts and perror)
#include <stdlib.h>   // Standard library
#include <unistd.h>

// The interface for a GPIO resource manager tailored for the Raspberry Pi's GPIO pins under QNX.
// This file provides functions to configure GPIO pins, set pin modes, and read/write pin values.
#include "rpi_gpio.h"
 
// Define the pins required to controll the motor driver
#define GPIO_LED_PIN       20 // GPIO pin 20, Controls the LED
#define GPIO_PIR_PIN       21 // GPIO pin 21, Takes input from the PIR sensor

bool running = true;

// handlers for intercepting Ctrl-C to stop application
static void ctrl_c_handler(int signum)
{
    (void)(signum);
    running = false;
}

static void setup_handlers(void)
{
    struct sigaction sa =
        {
            .sa_handler = ctrl_c_handler,
        };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main(void) {
    setup_handlers();

    // Initialize the LED GPIO pin to be an output
    if (rpi_gpio_setup(GPIO_LED_PIN, GPIO_OUT))
    {
        perror("rpi_gpio_setup: LED");
        exit(1);
    }

    // Initialize the PIR GPIO pin to be an input
    if (rpi_gpio_setup(GPIO_PIR_PIN, GPIO_IN))
    {
        perror("rpi_gpio_setup: PIR");
        exit(2);
    }

    // The while loop continuously reads the data from the PIR sensor and turns the LED on if motion is detected
    unsigned pin_level = 0;

    while(running){
        // 'rpi_gpio_read' reads the value from the PIR sensor when called
        // The return value is GPIO_HIGH, if motion is detected and GPIO_LOW otherwise
        // If the return is GPIO_HIGH we turn on the LED light
        if (!rpi_gpio_input(GPIO_PIR_PIN, &pin_level)) {
            if (pin_level == GPIO_HIGH) {
                if (!rpi_gpio_output(GPIO_LED_PIN, GPIO_HIGH)) {     // Turn on the LED light
                    printf("Motion Detected!!!\n");
                }
            } else if (pin_level == GPIO_LOW) {
                if (!rpi_gpio_output(GPIO_LED_PIN, GPIO_LOW)) {
                    printf("No Motion\n");
                }
            }
        }

        usleep(100000); // Sleep for 100 ms
    }

    if (rpi_gpio_output(GPIO_LED_PIN, GPIO_LOW)) {
        perror("Failed to turn off LED");
    }

    if (rpi_gpio_cleanup())
    {
        perror("Failed to cleanup GPIO");
    }

    printf("Exiting ...\n");

    return EXIT_SUCCESS;
}
