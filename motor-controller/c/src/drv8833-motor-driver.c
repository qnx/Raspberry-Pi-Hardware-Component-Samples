/******************************************************************************
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
* @file drv8833-motor-driver.c
* @brief Motor Driver Example for controlling motor speed and direction using PWM.
*
* This example uses the rpi_gpio resource manager to set up a PWM output on
* the motor driver's enable pin (EN) so that motor speed can be controlled.
* In this board design, the DRV8833 motor driver's speed is controlled via a PWM
* signal on the enable (nSLEEP) pin, while the other four pins determine the motor
* direction.
*
* IMPORTANT:
* The motor enable (EN) pin MUST be configured for PWM output. Without proper PWM
* configuration, the motor speed will not vary because the enable pin will not modulate
* the power supplied to the motors.
*
* This code continuously updates the PWM duty cycle (using rpi_gpio_set_pwm_duty_cycle)
* to "ramp" the speed from 0% to 100% and back while the motors run forward.
*
*****************************************************************************/
 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>     
#include "rpi_gpio.h"   // Raspberry Pi GPIO functions 
 
// GPIO pin definitions
/* NOTE: The MOTOR_EN_PIN (enable pin) MUST be configured for PWM so that
   motor speeds can be controlled by changing the duty cycle.*/
#define MOTOR_EN_PIN    18   ///< DRV8833 Enable/nSLEEP pin (PWM controlled)
#define MOTOR_RP_PIN    10   ///< Right Motor "Forward" input (AIN1)
#define MOTOR_RN_PIN    9    ///< Right Motor "Reverse" input (AIN2)
#define MOTOR_LP_PIN    8    ///< Left Motor "Forward" input (BIN1)
#define MOTOR_LN_PIN    11   ///< Left Motor "Reverse" input (BIN2)
 
// PWM frequency (in Hz) via rpi_gpio.
#define PWM_FREQ 1000
 
/**
* @brief Configures a GPIO pin for PWM output.
*
* This function sets up the specified GPIO pin as a PWM output at the defined
* PWM frequency using Mark/Space mode. It also sets the initial PWM duty cycle to 0%.
*
* @param gpio_pin The GPIO pin number to configure for PWM.
* @return int Returns GPIO_SUCCESS on success, or an error code if configuration fails.
*/
static int setup_pwm_pin(int gpio_pin)
{
    int rc;
 
    // Set up the GPIO pin as an output.
    rc = rpi_gpio_setup(gpio_pin, GPIO_OUT);
    if (rc != GPIO_SUCCESS) {
        printf("ERROR: rpi_gpio_setup() failed for pin %d, rc=%d\n", gpio_pin, rc);
        return rc;
    }
 
    // Initialize PWM on this pin using Mark/Space mode at PWM_FREQ.
    rc = rpi_gpio_setup_pwm(gpio_pin, PWM_FREQ, GPIO_PWM_MODE_MS);
    if (rc != GPIO_SUCCESS) {
        printf("ERROR: rpi_gpio_setup_pwm() failed for pin %d, rc=%d\n", gpio_pin, rc);
        return rc;
    }
 
    // Set the initial PWM duty cycle to 0% (motor off).
    rc = rpi_gpio_set_pwm_duty_cycle(gpio_pin, 0.0f);
    if (rc != GPIO_SUCCESS) {
        printf("ERROR: rpi_gpio_set_pwm_duty_cycle() failed for pin %d, rc=%d\n", gpio_pin, rc);
        return rc;
    }
 
    return GPIO_SUCCESS;
}
 
/**
* @brief Initializes the motor driver hardware.
*
* This function configures the motor enable pin for PWM output and sets up the
* direction control pins as digital outputs. All direction pins are initially
* set to a safe state (LOW).
*
* @return int Returns GPIO_SUCCESS on success, or an error code if initialization fails.
*/
int motor_init(void)
{
    int rc;
    printf("[motor_init] Initializing motor driver...\n");
 
    // Set up the enable pin for PWM control.
    // IMPORTANT: MOTOR_EN_PIN must be configured for PWM to allow speed control.
    rc = setup_pwm_pin(MOTOR_EN_PIN);
    if (rc != GPIO_SUCCESS)
        return rc;
 
    // Set up the motor direction pins as digital outputs.
    if (rpi_gpio_setup(MOTOR_RP_PIN, GPIO_OUT) != GPIO_SUCCESS ||
        rpi_gpio_setup(MOTOR_RN_PIN, GPIO_OUT) != GPIO_SUCCESS ||
        rpi_gpio_setup(MOTOR_LP_PIN, GPIO_OUT) != GPIO_SUCCESS ||
        rpi_gpio_setup(MOTOR_LN_PIN, GPIO_OUT) != GPIO_SUCCESS) {
        printf("ERROR: Failed to setup one or more direction pins.\n");
        return -1;
    }
 
    // Initialize all direction pins to LOW to ensure the motors are off.
    rpi_gpio_output(MOTOR_RP_PIN, GPIO_LOW);
    rpi_gpio_output(MOTOR_RN_PIN, GPIO_LOW);
    rpi_gpio_output(MOTOR_LP_PIN, GPIO_LOW);
    rpi_gpio_output(MOTOR_LN_PIN, GPIO_LOW);
 
    printf("[motor_init] Motor driver initialization complete.\n");
    return GPIO_SUCCESS;
}
 
/**
* @brief Sets the motor speed by updating the PWM duty cycle.
*
* This function adjusts the PWM duty cycle on the motor enable pin, thereby
* controlling the motor speed. The speed is specified as a percentage (0.0 to 100.0).
*
* @param speed_percent The desired speed as a percentage (0.0 = off, 100.0 = full speed).
* @return int Returns GPIO_SUCCESS on success, or an error code if setting the duty cycle fails.
*/
int motor_set_speed(float speed_percent)
{
    // Set the PWM duty cycle on the enable pin to control motor speed.
    return rpi_gpio_set_pwm_duty_cycle(MOTOR_EN_PIN, speed_percent);
}
 
/**
* @brief Disables the motor driver.
*
* This function stops the motor by setting the PWM duty cycle on the enable pin to 0%.
*
* @return int Returns GPIO_SUCCESS on success.
*/
int motor_disable(void)
{
    // Disable the motor by setting the PWM duty cycle to 0%.
    return motor_set_speed(0.0f);
}
 
/**
* @brief Signal handler for SIGINT (Ctrl+C).
*
* This handler ensures that the motor is disabled before the program exits.
*
* @param sig The signal number (unused).
*/
void sigint_handler(int sig)
{
    (void)sig;  // Unused parameter
    printf("\nSIGINT received, disabling motor driver...\n");
    motor_disable();
    exit(EXIT_SUCCESS);
}
 
/******************************************************************************
* Direction Functions
*
* The following functions control the direction of the motors by setting the appropriate
* GPIO pins to HIGH or LOW. Note that for the left motor, the wiring is reversed.
*
* Right Motor:
*   - Forward: MOTOR_RP_PIN = HIGH, MOTOR_RN_PIN = LOW.
*   - Reverse: MOTOR_RP_PIN = LOW, MOTOR_RN_PIN = HIGH.
*
* Left Motor (reversed wiring):
*   - Forward: MOTOR_LP_PIN = LOW, MOTOR_LN_PIN = HIGH.
*   - Reverse: MOTOR_LP_PIN = HIGH, MOTOR_LN_PIN = LOW.
*****************************************************************************/
 
/**
* @brief Sets the right motor to move forward.
*
* @return int Returns GPIO_SUCCESS on success.
*/
int motor_right_forward(void)
{
    rpi_gpio_output(MOTOR_RP_PIN, GPIO_HIGH);
    rpi_gpio_output(MOTOR_RN_PIN, GPIO_LOW);
    return GPIO_SUCCESS;
}
 
/**
* @brief Sets the right motor to move in reverse.
*
* @return int Returns GPIO_SUCCESS on success.
*/
int motor_right_reverse(void)
{
    rpi_gpio_output(MOTOR_RP_PIN, GPIO_LOW);
    rpi_gpio_output(MOTOR_RN_PIN, GPIO_HIGH);
    return GPIO_SUCCESS;
}
 
/**
* @brief Sets the left motor to move forward.
*
* @return int Returns GPIO_SUCCESS on success.
*/
int motor_left_forward(void)
{
    rpi_gpio_output(MOTOR_LP_PIN, GPIO_LOW);
    rpi_gpio_output(MOTOR_LN_PIN, GPIO_HIGH);
    return GPIO_SUCCESS;
}
 
/**
* @brief Sets the left motor to move in reverse.
*
* @return int Returns GPIO_SUCCESS on success.
*/
int motor_left_reverse(void)
{
    rpi_gpio_output(MOTOR_LP_PIN, GPIO_HIGH);
    rpi_gpio_output(MOTOR_LN_PIN, GPIO_LOW);
    return GPIO_SUCCESS;
}
 
/**
* @brief Stops both motors.
*
* This function sets all motor direction pins to LOW and disables the motor speed.
*
* @return int Returns GPIO_SUCCESS on success.
*/
int motor_stop(void)
{
    rpi_gpio_output(MOTOR_RP_PIN, GPIO_LOW);
    rpi_gpio_output(MOTOR_RN_PIN, GPIO_LOW);
    rpi_gpio_output(MOTOR_LP_PIN, GPIO_LOW);
    rpi_gpio_output(MOTOR_LN_PIN, GPIO_LOW);
    motor_set_speed(0.0f);
    return GPIO_SUCCESS;
}
 
/******************************************************************************
* Turning Functions (Pivot Turns)
*
* The following functions allow the robot to perform pivot turns by setting the motors
* in opposite directions.
*
* For a left turn: left motor reverse, right motor forward.
* For a right turn: left motor forward, right motor reverse.
*****************************************************************************/
 
/**
* @brief Pivots the robot to the left.
*
* @return int Returns GPIO_SUCCESS on success.
*/
int motor_turn_left(void)
{
    motor_left_reverse();
    motor_right_forward();
    return GPIO_SUCCESS;
}
 
/**
* @brief Pivots the robot to the right.
*
* @return int Returns GPIO_SUCCESS on success.
*/
int motor_turn_right(void)
{
    motor_left_forward();
    motor_right_reverse();
    return GPIO_SUCCESS;
}
 
/**
* @brief Main function to demonstrate motor control.
*
* This test routine sets the motors to drive forward and then ramps the PWM duty cycle
* (using motor_set_speed) from 0% to 100% and back down, demonstrating variable speed control.
* It also performs reverse drive and pivot turn operations.
*
* @return int Returns EXIT_SUCCESS if the program completes successfully.
*/
int main(void)
{
    int speed;
 
    // SIGINT handler to catch Ctrl+C and disable the motors safely.
    signal(SIGINT, sigint_handler);
 
    printf("=== DRV8833 Motor Driver PWM Demo (Enable Pin Speed Control) ===\n");
 
    // Initialize the motor driver.
    if (motor_init() != GPIO_SUCCESS) {
        printf("ERROR: Motor initialization failed.\n");
        return EXIT_FAILURE;
    }
 
    // Set both motors for forward drive.
    motor_right_forward();
    motor_left_forward();
 
    printf("\n--- Ramping speed from 0%% to 100%% ---\n");
    for (speed = 0; speed <= 100; speed += 10) {
        printf("Setting speed to %d%%\n", speed);
        motor_set_speed((float)speed);
        sleep(2);  
    }
 
    printf("\n--- Ramping speed from 100%% down to 0%% ---\n");
    for (speed = 100; speed >= 0; speed -= 10) {
        printf("Setting speed to %d%%\n", speed);
        motor_set_speed((float)speed);
        sleep(2);
    }
 
    // Reverse drive test.
    printf("\n--- Driving reverse at 50%% speed ---\n");
    motor_right_reverse();
    motor_left_reverse();
    motor_set_speed(50.0f);
    sleep(2);
    motor_stop();
    sleep(1);
 
    // Pivot turn: Left turn.
    printf("\n--- Pivot Turning LEFT at 30%% speed ---\n");
    motor_turn_left();
    motor_set_speed(30.0f);
    sleep(2);
    motor_stop();
    sleep(1);
 
    // Pivot turn: Right turn.
    printf("\n--- Pivot Turning RIGHT at 30%% speed ---\n");
    motor_turn_right();
    motor_set_speed(30.0f);
    sleep(2);
    motor_stop();
    sleep(1);
 
    printf("\n--- Disabling motor driver ---\n");
    motor_disable();
 
    printf("=== Test Complete ===\n");
    return EXIT_SUCCESS;
}
