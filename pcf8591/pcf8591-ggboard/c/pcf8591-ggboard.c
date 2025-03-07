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
#define PCF8591_AIN0 0x40 // input 0 (built in 10K pot connected with jumper across pins)
#define PCF8591_AIN1 0x41 // input 1 (built in photoresistor 5537 connected with jumper across pins)
#define PCF8591_AIN2 0x42 // input 2 (built in thermistor MF58 connected with jumper across pins)
#define PCF8591_AIN3 0x43 // input 3 (grounded reads as 0 with jumper across pins)
#define PCF8591_AIN_ALL 0x44 // all inputs with auto increment flag
#define PCF8591_AOU0 0x40


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
    uint8_t values[4] = {0};

    // When using auto increment, you get inputs 1, 2, 3, and then 0, so this lookup table
    // compensates for that so that values can be printed in the expected input order
    const unsigned autoIncrementInputIndexes[4] = {1, 2, 3, 0};

    while (running)
    {
        // Read the inputs individually first
        // Individual reads are doubled up to get correct values.  This could be a side effect of batch
        // reading with the autoincrement flag or due to a i2c driver issue.
        if (smbus_read_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN0, &value))
        {
            return EXIT_FAILURE;
        }
        if (smbus_read_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN0, &value))
        {
            return EXIT_FAILURE;
        }
        printf("New 10K POT value: %d\n", value);

        if (smbus_read_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN1, &value))
        {
            return EXIT_FAILURE;
        }
        if (smbus_read_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN1, &value))
        {
            return EXIT_FAILURE;
        }
        printf("New 5537 Photoresistor value: %d\n", value);

        if (smbus_read_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN2, &value))
        {
            return EXIT_FAILURE;
        }
        if (smbus_read_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN2, &value))
        {
            return EXIT_FAILURE;
        }
        printf("New MF58 Thermistor value: %d\n", value);

        if (smbus_read_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN3, &value))
        {
            return EXIT_FAILURE;
        }
        if (smbus_read_byte_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN3, &value))
        {
            return EXIT_FAILURE;
        }
        printf("New AIN3 value: %d\n", value);

        // Read the inputs all at once with autoincrement
        if (smbus_read_block_data(PI_SMB_BUS, PCF8591_ADDRESS, PCF8591_AIN_ALL, values, 4))
        {
            return EXIT_FAILURE;
        }
        printf("New input values: ");
        for (int index1 = 0; index1 < 4; index1++)
        {
            printf(" %d", values[autoIncrementInputIndexes[index1]]);
        }
        printf("\n");

        nanosleep(&wait_interval_time_spec, NULL);
    }

    if (smbus_cleanup(PI_SMB_BUS))
    {
        return EXIT_FAILURE;
    }

    printf("Exiting ...\n");

    return EXIT_SUCCESS;
}
