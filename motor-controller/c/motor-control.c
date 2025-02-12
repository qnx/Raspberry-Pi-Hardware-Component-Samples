#include <stdio.h>    // Standard input/output functions (for puts and perror)
#include <stdlib.h>   // Standard library (for EXIT_SUCCESS and EXIT_FAILURE)
 
// The interface for a GPIO resource manager tailored for the Raspberry Pi's GPIO pins under QNX.
// This file provides functions to configure GPIO pins, set pin modes, and read/write pin values.
#include "rpi_gpio.h"
 
// This global variable is used to store a pointer to the Raspberry Pi's GPIO registers.
// It is declared as volatile to prevent compiler optimizations that may interfere with hardware access.
volatile uint32_t *rpi_gpio_regs;


// Define the pins required to controll the motor driver
#define MOTOR_EN_PIN   26 // GPIO pin 26, Enable pin is used to turn on and off the motors
#define MOTOR_RP       10 // GPIO pin 10, Controls the input to the positive terminal off the right motor
#define MOTOR_RN        9 // GPIO pin 9, Controls the input to the negative terminal off the right motor
#define MOTOR_LP        8 // GPIO pin 8, Controls the input to the positive terminal off the left motor
#define MOTOR_LN       11 // GPIO pin 11, Controls the input to the negative terminal off the left motor

// Base address of the GPIO registers on the Raspberry Pi
#define RPI_PERIPHERAL_BASE 0xfe000000


int main(void) {
    // Map the physical GPIO register memory into our program's address space.
    // This is required by 'rpi_gpio.h' function to interact with the GPIO hardware.
    // The if statement checks for any errors in setting up our pins
    if (!rpi_gpio_map_regs(RPI_PERIPHERAL_BASE)) {
        printf("Failed to map GPIO registers.\n");
        return false;
    }

    // Initialize all of the defined pins to be outputs
    rpi_gpio_set_select(MOTOR_EN_PIN, RPI_GPIO_FUNC_OUT);
    rpi_gpio_set_select(MOTOR_RP,     RPI_GPIO_FUNC_OUT);
    rpi_gpio_set_select(MOTOR_RN,     RPI_GPIO_FUNC_OUT);
    rpi_gpio_set_select(MOTOR_LP,     RPI_GPIO_FUNC_OUT);
    rpi_gpio_set_select(MOTOR_LN,     RPI_GPIO_FUNC_OUT);

    // Set our enable to pin to an on state allowing our motors to move
    rpi_gpio_set(MOTOR_EN_PIN);

    // Move the motors in a forward direction
    // To move the motors forward, the positive terminal of the left and right motors needs to be set
    // Likewise the negative terminal of the left and right motors needs to be cleared
    rpi_gpio_set(MOTOR_RP);
    rpi_gpio_clear(MOTOR_RN);
    rpi_gpio_set(MOTOR_LP);
    rpi_gpio_clear(MOTOR_LN);

    // We keep the state for 2 seconds, allowing our motors to move forward for that time
    printf("Moving Forward...\n");
    sleep(2);

    // Move the motors in a backward direction
    // To move the motors backward, the negative terminal of the left and right motors needs to be set
    // Likewise the positive terminal of the left and right motors needs to be cleared
    rpi_gpio_set(MOTOR_RP);
    rpi_gpio_clear(MOTOR_RP);
    rpi_gpio_set(MOTOR_RN);
    rpi_gpio_clear(MOTOR_LP);
    rpi_gpio_set(MOTOR_LN);
    
    // We keep the state for 2 seconds, allowing our motors to move backward for that time
    printf("Moving Backward...\n");
    sleep(2);

    // Move the motors in a left direction
    // To move left, the right motor needs to be going forward, while the left needs to move backwards
    // For this we set the positive terminal of the right motor and the negative terminal of the left motor
    // Likewise we clear the negative terminal of the right motor and the positive terminal of the left motor
    rpi_gpio_set(MOTOR_RP);
    rpi_gpio_clear(MOTOR_RN);
    rpi_gpio_clear(MOTOR_LP);
    rpi_gpio_set(MOTOR_LN);
    
    // We keep the state for 2 seconds, allowing our motors to move left for that time
    printf("Moving Left...\n");
    sleep(2);

    // Move the motors in a right direction
    // To move right, the right motor needs to be going backwards, while the left needs to move forward
    // For this we set the positive terminal of the left motor and the negative terminal of the right motor
    // Likewise we clear the negative terminal of the left motor and the positve terminal of the right motor
    rpi_gpio_clear(MOTOR_RP);
    rpi_gpio_set(MOTOR_RN);
    rpi_gpio_set(MOTOR_LP);
    rpi_gpio_clear(MOTOR_LN);
 
    printf("Moving Right...\n");
    sleep(2);
 
    // To stop our motors we clear all of the pins
    rpi_gpio_clear(MOTOR_EN_PIN);
    rpi_gpio_clear(MOTOR_RP);
    rpi_gpio_clear(MOTOR_RN);
    rpi_gpio_clear(MOTOR_LP);
    rpi_gpio_clear(MOTOR_LN);

    printf("Motors Stopped.\n");
 
    return 0;

}