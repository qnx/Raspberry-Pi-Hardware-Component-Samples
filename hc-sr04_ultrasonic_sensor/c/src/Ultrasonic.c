/**
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
*
* @file Ultrasonic.c
* @brief HC‑SR04 Ultrasonic Sensor Example for QNX.
*
* This example demonstrates how to interface with the HC‑SR04 ultrasonic sensor
* on a QNX system. It sends a trigger pulse, waits for the echo’s rising and falling
* edges, and then calculates the distance based on the pulse duration.
*
* Wiring:
*   - P1: 5V
*   - P2: US_TRIG   -> connected to GPIO 13
*   - P3: US_ECHO   -> connected to GPIO 25 (with built‑in resistor divider)
*   - P4: GND
*
* IMPORTANT:
* The sensor is powered by 5V, and the built‑in resistor divider reduces the echo
* signal voltage (approximately 2.5V when high). An internal pull‑DOWN is disabled
* on GPIO 25 to allow the sensor’s divider to work properly.
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/iomsg.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include "rpi_gpio.h"   // QNX inline functions for GPIO control

// GPIO pin definitions (using BCM numbering)
#define GPIO_PULSE_PIN 13  ///< GPIO pin for the trigger signal (US_TRIG)
#define GPIO_ECHO_PIN  25  ///< GPIO pin for the echo signal (US_ECHO)
 
// Speed of sound in cm per microsecond.
#define SPEED_OF_SOUND_CM_PER_US 0.0343
 
// Maximum time to wait for echo edges (in milliseconds).
#define EDGE_TIMEOUT_MS 50.0
 
// Peripheral base address (mapping adds 0x200000 so registers map to 0xFE200000)
#define RPI_PERIPHERAL_BASE 0xfe000000

// Define possible event types for the system
enum sample_event_t
{
    EVENT_ECHO, // Event triggered by button press
}; 

static int chid;  // Channel ID
static int coid;  // Connection ID

// Track the time of the last valid button press
static struct timespec last_trigger_event_time;

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
 
/**
* @brief Calculates the elapsed time in microseconds between two time points.
*
* @param start Pointer to the start timespec structure.
* @param stop Pointer to the stop timespec structure.
* @return float Elapsed time in microseconds.
*/
static float calculate_elapsed_time_us(const struct timespec *start, const struct timespec *stop)
{
    return (stop->tv_sec - start->tv_sec) * 1e6 +
           (stop->tv_nsec - start->tv_nsec) / 1e3;
}
 
/**
* @brief Sends a 10-microsecond pulse on the ultrasonic trigger pin.
*
* This function sets the trigger GPIO pin HIGH for 10 µs and then clears it.
*
* @return int Returns 0 on success.
*/
static bool send_pulse(void)
{
    printf("Sending trigger pulse...\n");
    if (rpi_gpio_output(GPIO_PULSE_PIN, GPIO_HIGH))
    {
        perror("rpi_gpio_output: start pulse");
        return false;
    }
 
    struct timespec pulse_duration = { .tv_sec = 0, .tv_nsec = 10 * 1000 };  // 10 µs
    nanosleep(&pulse_duration, NULL);
 
    if (rpi_gpio_output(GPIO_PULSE_PIN, GPIO_LOW))
    {
        perror("rpi_gpio_output: end pulse");
        return false;
    }

    return true;
}
 
/**
* @brief Initializes the GPIO pins for use with the ultrasonic sensor.
*
* This function maps the GPIO registers and configures:
*   - The trigger pin (US_TRIG) as an output.
*   - The echo pin (US_ECHO) as an input.
* It also disables the internal pull resistor on the echo pin.
*
* @return bool Returns true if the initialization is successful, or false otherwise.
*/
static bool init_gpios(void)
{
    // Configure the trigger pin (US_TRIG) as an output.
    if (rpi_gpio_setup(GPIO_PULSE_PIN, GPIO_OUT))
    {
        perror("rpi_gpio_setup: PULSE");
        return false;
    }

    // Configure the echo pin (US_ECHO) as an input.
    if (rpi_gpio_setup_pull(GPIO_ECHO_PIN, GPIO_IN, GPIO_PUD_OFF))
    {
        perror("Failed to disable pull resistor on echo pin");
        return false;
    }

    // Register an event trigger on a falling edge (button press)
    if (rpi_gpio_add_event_detect(GPIO_ECHO_PIN, coid, GPIO_RISING | GPIO_FALLING, EVENT_ECHO))
    {
        perror("rpi_gpio_add_event_detect");
        return false;
    }
 
    return true;
}
 
/**
* @brief Reads the distance measured by the ultrasonic sensor.
*
* This function sends a trigger pulse and then polls for the echo pin’s rising and
* falling edges. It computes the pulse duration and calculates the distance based on
* the speed of sound.
*
* @param distance Pointer to a float where the computed distance (in cm) will be stored.
* @return int Returns 0 on success, or -1 if an error occurs (e.g., timeout).
*/
static int check_distance(float *distance)
{
    // send pulse when distance is set to negative value
    if (*distance < 0)
    {
        if (!send_pulse() != 0)
        {
            printf("Failed to send trigger pulse\n");
            return -1;
        }

        *distance = 0.0;

        return 0;
    }
 
    struct timespec current_time;

    // Get the current time
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    if (last_trigger_event_time.tv_nsec == 0)
    {
        printf("Rising edge detected\n");

        last_trigger_event_time = current_time;

        return 1;
    } else
    {
        printf("Falling edge detected\n");

        // Calculate the echo pulse duration in microseconds.
        float pulse_duration_us = calculate_elapsed_time_us(&last_trigger_event_time, &current_time);
 
        // Compute the distance in centimeters (divide by 2 for the round-trip).
        *distance = (pulse_duration_us * SPEED_OF_SOUND_CM_PER_US) / 2.0;

        // reset last trigger time
        last_trigger_event_time.tv_sec = 0;
        last_trigger_event_time.tv_nsec = 0;

        return 0;
    }
 
    return -1;
}
 
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

/**
* @brief Main function for the ultrasonic sensor example.
*
* Sets the clock period, initializes the GPIO pins, and continuously reads and prints
* the measured distance.
*
* @return int Returns EXIT_SUCCESS on normal termination, or EXIT_FAILURE on error.
*/
int main(void)
{
    setup_handlers();

    // Initialize the communication channel
    if (!init_channel())
    {
        return EXIT_FAILURE;
    }
 
    // Initialize the GPIOs.
    if (!init_gpios())
    {
        perror("Failed to initialize GPIOs.");
        return EXIT_FAILURE;
    }

    // Allow the sensor to settle.
    sleep(2);
 
    last_trigger_event_time.tv_sec = 0;
    last_trigger_event_time.tv_nsec = 0;

    int return_code;
    float distance = -1.0;
    struct _pulse pulse;

    // Continuously measure and print the distance.

    // send pulse
    if (check_distance(&distance))
    {
        perror("Error sending pulse.");
    }
 
    while(running)
    {
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
            case EVENT_ECHO:
                return_code = check_distance(&distance);

                switch (return_code)
                {
                    case 0:
                        printf("Distance: %.2f cm\n", distance);
 
                        usleep(100000);  // Delay 100 ms between measurements.

                        // send next pulse
                        last_trigger_event_time.tv_sec = 0;
                        last_trigger_event_time.tv_nsec = 0;
                        distance = -1.0;
                        if (check_distance(&distance))
                        {
                            perror("Error sending pulse.");
                        }
				        break;

                    // do nothing
                    case 1:
                        printf("Received one event, waiting for other ...\n");
				        break;

                    case -1:
                        printf("Error reading distance\n");
				        break;
                }
				break;
        }
    }
 
    if (rpi_gpio_cleanup())
    {
        perror("Failed to cleanup GPIOs.");
    }

    printf("Exiting ...\n");
 
    return EXIT_SUCCESS;
}
