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

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include "rpi_i2c.h"

// PCF8591 constants
#define PI_SMB_BUS 1
#define PCF8591_ADDRESS 0x48
#define PCF8591_AIN0 0x40 // input 0
#define PCF8591_AOU0 0x40 // output 0 (same address as input 0)

bool running = true;

// handlers for intercepting Ctrl-C to stop application
static void ctrl_c_handler(int signum)
{
    (void)(signum);
    running = 0;
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

int main(int argc, char **argv)
{
    setup_handlers();

    struct timespec wait_interval_time_spec = {.tv_nsec = 10000000};
    uint8_t value = 0;
    uint8_t prev_value = 0;

    while (running)
    {
        // read the port from the PCF8591 ADC AIN0
        if (smbus_read_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN0, &value)) // built in 10K pot or external pot 1
        {
            return EXIT_FAILURE;
        }

        if (prev_value != value)
        {
            printf("New 10K POT value: %d\n", value);

	    prev_value = value;

            // output input value from 10K pot to analog out to control voltage to the LED
            if (smbus_write_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AOU0, value)) // single DAC output
            {
                return EXIT_FAILURE;
            }
            printf("Outputting value to DAC (255 = 3.3V): %d\n", value);
        }

        nanosleep(&wait_interval_time_spec, NULL);
    }

    if (smbus_cleanup(PI_SMB_BUS))
    {
        return EXIT_FAILURE;
    }

    printf("Exiting ...\n");

    return EXIT_SUCCESS;
}
