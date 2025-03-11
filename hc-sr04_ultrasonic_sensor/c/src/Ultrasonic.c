/**
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
*
* Run this code with root privileges.
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include "rpi_gpio.h"   // QNX inline functions for GPIO control

 
// GPIO pin definitions (using BCM numbering)
#define GPIO_PULSE_PIN     13  ///< GPIO pin for the trigger signal (US_TRIG)
#define GPIO_INTERRUPT_PIN 25  ///< GPIO pin for the echo signal (US_ECHO)
 
// Speed of sound in cm per microsecond.
#define SPEED_OF_SOUND_CM_PER_US 0.0343
 
// Maximum time to wait for echo edges (in milliseconds).
#define EDGE_TIMEOUT_MS 50.0
 
// Peripheral base address (mapping adds 0x200000 so registers map to 0xFE200000)
#define RPI_PERIPHERAL_BASE 0xfe000000
 
/// Global pointer required by the inline functions for accessing GPIO registers.
volatile uint32_t *__RPI_GPIO_REGS = NULL;
 
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
* @brief Waits for a specified GPIO pin to reach the desired state.
*
* This function polls the given GPIO pin until it reaches the desired state
* (HIGH if desired_state is true, LOW if false) or until the timeout expires.
*
* @param gpio_pin The GPIO pin number to monitor.
* @param desired_state The state to wait for (true for HIGH, false for LOW).
* @param timeout_ms Maximum time to wait in milliseconds.
* @param timestamp Pointer to a timespec structure where the detection time will be stored.
* @return int Returns 0 if the state is detected within the timeout, or -1 if a timeout occurs.
*/
static int wait_for_gpio_state(int gpio_pin, bool desired_state, double timeout_ms, struct timespec *timestamp)
{
    struct timespec start, current;
    clock_gettime(CLOCK_REALTIME, &start);
 
    while (1)
    {
        bool current_state = (rpi_gpio_read(gpio_pin) != 0);
        if (current_state == desired_state)
        {
            clock_gettime(CLOCK_REALTIME, timestamp);
            return 0;
        }
        clock_gettime(CLOCK_REALTIME, &current);
        double elapsed_ms = (current.tv_sec - start.tv_sec) * 1000.0 +
                            (current.tv_nsec - start.tv_nsec) / 1e6;
        if (elapsed_ms > timeout_ms)
        {
            return -1;
        }
    }
}
 
/**
* @brief Sends a 10-microsecond pulse on the ultrasonic trigger pin.
*
* This function sets the trigger GPIO pin HIGH for 10 µs and then clears it.
*
* @return int Returns 0 on success.
*/
static int send_pulse(void)
{
    printf("Sending trigger pulse...\n");
    rpi_gpio_set(GPIO_PULSE_PIN);
 
    struct timespec pulse_duration = { .tv_sec = 0, .tv_nsec = 10 * 1000 };  // 10 µs
    nanosleep(&pulse_duration, NULL);
 
    rpi_gpio_clear(GPIO_PULSE_PIN);
    return 0;
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
    // Map the GPIO registers.
    if (!rpi_gpio_map_regs(RPI_PERIPHERAL_BASE))
    {
        printf("Failed to map GPIO registers\n");
        return false;
    }
 
    // Configure the trigger pin (US_TRIG) as an output.
    rpi_gpio_set_select(GPIO_PULSE_PIN, RPI_GPIO_FUNC_OUT);
 
    // Configure the echo pin (US_ECHO) as an input.
    rpi_gpio_set_select(GPIO_INTERRUPT_PIN, RPI_GPIO_FUNC_IN);
 
    // Disable the internal pull resistor on the echo pin.
    if (!rpi_gpio_set_pud_bcm2711(GPIO_INTERRUPT_PIN, RPI_GPIO_PUD_OFF))
    {
        printf("Failed to disable pull resistor on echo pin\n");
        return false;
    };
 
    // Clear any previous GPIO events.
    __RPI_GPIO_REGS[RPI_GPIO_REG_GPEDS0] = 0xFFFFFFFF;
 
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
static int read_distance(float *distance)
{
    if (send_pulse() != 0)
    {
        printf("Failed to send trigger pulse\n");
        return -1;
    }
 
    struct timespec rising_edge_time, falling_edge_time;
 
    // Wait for the rising edge on the echo pin.
    if (wait_for_gpio_state(GPIO_INTERRUPT_PIN, true, EDGE_TIMEOUT_MS, &rising_edge_time) != 0)
    {
        printf("Timeout waiting for rising edge\n");
        return -1;
    }
    printf("Rising edge detected\n");
 
    // Wait for the falling edge on the echo pin.
    if (wait_for_gpio_state(GPIO_INTERRUPT_PIN, false, EDGE_TIMEOUT_MS, &falling_edge_time) != 0)
    {
        printf("Timeout waiting for falling edge\n");
        return -1;
    }
    printf("Falling edge detected\n");
 
    // Calculate the echo pulse duration in microseconds.
    float pulse_duration_us = calculate_elapsed_time_us(&rising_edge_time, &falling_edge_time);
 
    // Compute the distance in centimeters (divide by 2 for the round-trip).
    *distance = (pulse_duration_us * SPEED_OF_SOUND_CM_PER_US) / 2.0;
 
    return 0;
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
    // Set a 10 µs clock period.
    struct _clockperiod period;
    period.nsec = 10000;  // 10 µs
    period.fract = 0;
    if (ClockPeriod(CLOCK_REALTIME, NULL, &period, 0) == -1)
    {
        perror("ClockPeriod");
        return EXIT_FAILURE;
    }
 
    // Initialize the GPIOs.
    if (!init_gpios())
    {
        printf("Failed to initialize GPIOs\n");
        return EXIT_FAILURE;
    }
 
    // Allow the sensor to settle.
    sleep(2);
 
    // Continuously measure and print the distance.
    while (1)
    {
        float distance;
        if (read_distance(&distance) == 0)
        {
            printf("Distance: %.2f cm\n", distance);
        }
        else
        {
            printf("Error reading distance\n");
        }
 
        usleep(100000);  // Delay 100 ms between measurements.
    }
 
    return EXIT_SUCCESS;
}
