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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/iomsg.h>
#include "rpi_gpio.h"


// define possible events types
enum sample_event_t
{
    EVENT_BUTTON_1,
};

// define GPIO pin number
#define RELAY_GPIO_PIN GPIO17
#define BUTTON_GPIO_PIN GPIO19

static int chid; // channel ID
static int coid; // connection ID

static bool init_channel(void)
{
    chid = ChannelCreate(_NTO_CHF_PRIVATE);
    if (chid == -1)
    {
        perror("ChannelCreate");
        return false;
    }

    coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1)
    {
        perror("ConnectAttach");
        return false;
    }
    return true;
}

static bool init_button_input(int gpio_pin, int button_event_id)
{
    if (rpi_gpio_setup_pull(gpio_pin, GPIO_IN, GPIO_PUD_DOWN))
    {
        perror("rpi_gpio_setup_pull");
        return false;
    }
    if (rpi_gpio_add_event_detect(gpio_pin, coid, GPIO_RISING | GPIO_FALLING, button_event_id))
    {
        perror("rpi_gpio_add_event_detect");
        return false;
    }
    return true;
}

static bool init_relay(int gpio_pin)
{
    if (rpi_gpio_setup(gpio_pin, GPIO_OUT))
    {
        perror("rpi_gpio_setup");
        return false;
    }
    return true;
}

// Set relay GPIO level
static bool relay_set(int gpio_pin, int level)
{
    if (rpi_gpio_output(gpio_pin, level))
    {
        perror("rpi_gpio_output");
        return false;
    }
    return true;
}

int main()
{

    // initializing the communication channel
    if (!init_channel())
    {
        return EXIT_FAILURE;
    }

    // initializing the button input
    if (!init_button_input(BUTTON_GPIO_PIN, EVENT_BUTTON_1))
    {
        return EXIT_FAILURE;
    }

    // initialize RELAY
    if (!init_relay(RELAY_GPIO_PIN))
    {
        return EXIT_FAILURE;
    }

    // initialize RELAY state level to low
    int relay_level = GPIO_LOW;

    // start with RELAY off
    if (!relay_set(RELAY_GPIO_PIN, GPIO_LOW))
    {
        return EXIT_FAILURE;
    }

    // track the time of the last valid button press
    struct timespec last_button_event_time;
    last_button_event_time.tv_sec = 0;
    last_button_event_time.tv_nsec = 0;

    // Bounce threshold in nanoseconds (e.g.,100ms)
    const long debounce_threshold = 100000000;

    for (;;)
    {
        struct _pulse pulse;
        struct timespec current_time;
        if (MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == -1)
        {
            perror("MsgReceivePulse()");
            return EXIT_FAILURE;
        }

        if (pulse.code != _PULSE_CODE_MINAVAIL)
        {
            fprintf(stderr, "Unexpected plse code %d\n", pulse.code);
            return EXIT_FAILURE;
        }

        switch (pulse.value.sival_int)
        {
        case EVENT_BUTTON_1:

            // get the current time
            clock_gettime(CLOCK_MONOTONIC, &current_time);

            long time_diff_ns =
                (current_time.tv_sec - last_button_event_time.tv_sec) * 1000000000L +
                (current_time.tv_nsec - last_button_event_time.tv_nsec);

            if (time_diff_ns < debounce_threshold)
            {
                break;
            }

            last_button_event_time = current_time;

            relay_level =
                (relay_level == GPIO_HIGH) ? GPIO_LOW : GPIO_HIGH;

            if (!relay_set(RELAY_GPIO_PIN, relay_level))
            {
                return EXIT_FAILURE;
            }

            printf("Magnet %s\n", relay_level == GPIO_HIGH ? "activated" : "deactivated");

            break;
        }
    }
    return EXIT_SUCCESS;
}
