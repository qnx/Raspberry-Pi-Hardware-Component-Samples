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

#include <fcntl.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include "rpi_gpio.h"

#define MIN_ANGLE_0_PERCENT 2.5
#define MAX_ANGLE_180_PERCENT 12.5

// Red wire to pin 2 (5V)
// Brown/Black wire to pin 6 (Ground)
// Orange/Yellow wire into GPIO 18 (pin 12)
#define GPIO_PIN GPIO18

// How much to step between each angle (in degrees)
#define DEGREES_STEP 10

// Duty cycle percentage for servo at 0-degree position
#define MIN_ANGLE_0_PERCENT 2.5
// Duty cycle percentage for servo at 180-degree position
#define MAX_ANGLE_180_PERCENT 12.5


/**
 * @brief Initializes a GPIO pin for servo control using PWM.
 *
 * This function configures the specified GPIO pin as a PWM output 
 * and sets the desired frequency for controlling a servo motor.
 *
 * @param gpio_pin The GPIO pin number connected to the servo.
 * @param frequency The PWM frequency in Hz.
 * @return true if the initialization was successful, false otherwise.
 */
static bool init_servo(int gpio_pin, unsigned frequency)
{
    // The mode must be set to M/S, as the control mechanism expects a continuous 
    // high level for the duty cycle in each period
    if (rpi_gpio_setup_pwm(gpio_pin, frequency, GPIO_PWM_MODE_MS))
    {
        perror("rpi_gpio_setup_pwm");
        return false;
    }

    return true;
}

/**
 * @brief Sets the servo motor to a specific angle using PWM.
 *
 * This function adjusts the PWM duty cycle of the specified GPIO pin 
 * to set the servo motor to the desired angle.
 *
 * @param gpio_pin The GPIO pin number connected to the servo.
 * @param angle The target angle for the servo (range: 0 to 180 degrees).
 * @return true if the PWM duty cycle was set successfully, false otherwise.
 */
static bool set_servo_angle(int gpio_pin, float angle)
{
    // Set selected GPIO PWM duty cycle
    if (rpi_gpio_set_pwm_duty_cycle(gpio_pin, 2.5 + (angle * (MAX_ANGLE_180_PERCENT - MIN_ANGLE_0_PERCENT) / 180.0)))
    {
        perror("rpi_gpio_set_pwm_duty_cycle");
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    if (!init_servo(GPIO_PIN, 50))
    {
        return EXIT_FAILURE;
    }

    // delay 1/2 second between angles
    struct timespec rotate_delay_interval_time_spec = {.tv_nsec = 500000000};

    // rotate servo 0 to 180 degrees
    int index = 0;
    for (index = 0; index <= 180; index += DEGREES_STEP)
    {
        if (!set_servo_angle(GPIO_PIN, (float)index))
        {
            return EXIT_FAILURE;
        }

        nanosleep(&rotate_delay_interval_time_spec, NULL);
    }

    // rotate servo in the other direction (180 to 0 degrees)
    for (index = 180; index >= 0; index -= DEGREES_STEP)
    {
        if (!set_servo_angle(GPIO_PIN, (float)index))
        {
            return EXIT_FAILURE;
        }

        nanosleep(&rotate_delay_interval_time_spec, NULL);
    }

    return EXIT_SUCCESS;
}
