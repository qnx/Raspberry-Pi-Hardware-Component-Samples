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
#include <stdlib.h>
#include "rpi_gpio.h"

// Define possible event types for the system
enum sample_event_t
{
    EVENT_BUTTON_1, // Event triggered by button press
};

// Define the GPIO pin numbers for LED and button
#define LED_GPIO_PIN GPIO16   // GPIO pin for LED output
#define BUTTON_GPIO_PIN GPIO20 // GPIO pin for button input

static int chid;  // Channel ID
static int coid;  // Connection ID

// Initializes the communication channel for event handling.
// This function creates a private message-passing channel (chid) for inter-process communication (IPC).
// The channel is used to receive pulse events from GPIO interrupts.
// A connection (coid) is then attached to this channel, allowing the process to send and receive events.
// If either step fails, the function returns false to indicate an initialization error.
static bool init_channel(void)
{
    // Create a private communication channel
    chid = ChannelCreate(_NTO_CHF_PRIVATE);
    if (chid == -1)
    {
        perror("ChannelCreate");
        return false;
    }

    // Attach a connection to the created channel
    coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1)
    {
        perror("ConnectAttach");
        return false;
    }

    return true;
}

// Initializes button input with an event listener
static bool init_button_input(int gpio_pin, int button_event_id)
{
    // Configure the button pin as input with a pull-up resistor
    if (rpi_gpio_setup_pull(gpio_pin, GPIO_IN, GPIO_PUD_UP))
    {
        perror("rpi_gpio_setup_pull");
        return false;
    }

    // Register an event trigger on a falling edge (button press)
    if (rpi_gpio_add_event_detect(gpio_pin, coid, GPIO_RISING | GPIO_FALLING, button_event_id))
    {
        perror("rpi_gpio_add_event_detect");
        return false;
    }

    return true;
}

/* LED control functions */
// Initializes the LED GPIO pin as an output
static bool init_led(int gpio_pin)
{
    if (rpi_gpio_setup(gpio_pin, GPIO_OUT))
    {
        perror("rpi_gpio_setup");
        return false;
    }

    return true;
}

// Turns on the LED
static bool led_on(int gpio_pin)
{
    if (rpi_gpio_output(gpio_pin, GPIO_HIGH))
    {
        perror("rpi_gpio_output");
        return false;
    }

    return true;
}

// Turns off the LED
static bool led_off(int gpio_pin)
{
    if (rpi_gpio_output(gpio_pin, GPIO_LOW))
    {
        perror("rpi_gpio_output");
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    // Initialize the communication channel
    if (!init_channel())
    {
        return EXIT_FAILURE;
    }

    // Initialize the button input
    if (!init_button_input(BUTTON_GPIO_PIN, EVENT_BUTTON_1))
    {
        return EXIT_FAILURE;
    }

    // Initialize the LED
    if (!init_led(LED_GPIO_PIN))
    {
        return EXIT_FAILURE;
    }

    // Track LED state to prevent unnecessary operations
    bool led_is_on = false;
	// Start with the LED off
	if (!led_off(LED_GPIO_PIN))
	{
		return EXIT_FAILURE;
	}

    // Track the time of the last valid button press
    struct timespec last_button_event_time;
	last_button_event_time.tv_sec = 0;
	last_button_event_time.tv_nsec = 0;

	// Debounce threshold in nanoseconds (e.g., 100ms)
	const long debounce_threshold = 100000000;

    for (;;)
    {
        struct _pulse pulse;
        if (MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == -1)
        {
            perror("MsgReceivePulse()");
            return EXIT_FAILURE;
        }

        if (pulse.code != _PULSE_CODE_MINAVAIL)
        {
            fprintf(stderr, "Unexpected pulse code %d\n", pulse.code);
            return EXIT_FAILURE;
        }

        switch (pulse.value.sival_int)
        {
            case EVENT_BUTTON_1:

				// Get the current time
				struct timespec current_time;
				clock_gettime(CLOCK_MONOTONIC, &current_time);

				// We sometimes get duplicate events on button up
				// On the first button up we turn off the LED and then want to ignore future events that come quickly
				if (!led_is_on)
				{
					// Check if enough time has passed since the last button event (debounce)
					long time_diff_ns = (current_time.tv_sec - last_button_event_time.tv_sec) * 1000000000L + (current_time.tv_nsec - last_button_event_time.tv_nsec);
					if (time_diff_ns < debounce_threshold)
					{
						// Ignore this event, as it's too close to the last one
						break;
					}
				}

				// Update the last recorded button press time
				last_button_event_time = current_time;

				if (led_is_on)
				{
					// LED is on, turn it off
					if (!led_off(LED_GPIO_PIN))
					{
						return EXIT_FAILURE;
					}
					led_is_on = false;
				}
				else
				{
					// LED is off, turn it on
					if (!led_on(LED_GPIO_PIN))
					{
						return EXIT_FAILURE;
					}
					led_is_on = true;
				}
				break;
        }
    }

    return EXIT_SUCCESS;
}
