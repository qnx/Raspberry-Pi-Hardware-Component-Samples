
#include <stdio.h>    // Standard input/output functions (for puts and perror)
#include <stdlib.h>   // Standard library
 
// The interface for a GPIO resource manager tailored for the Raspberry Pi's GPIO pins under QNX.
// This file provides functions to configure GPIO pins, set pin modes, and read/write pin values.
#include "rpi_gpio.h"
 
// This global variable is used to store a pointer to the Raspberry Pi's GPIO registers.
// It is declared as volatile to prevent compiler optimizations that may interfere with hardware access.
volatile uint32_t *rpi_gpio_regs;

// Define the pins required to controll the motor driver
#define GPIO_LED_PIN       20 // GPIO pin 20, Controls the LED
#define GPIO_PIR_PIN       21 // GPIO pin 21, Takes input from the PIR sensor

// Base address of the GPIO registers on the raspberry pi
#define RPI_PERIPHERAL_BASE 0xfe000000

int main(void) {
    // Map the physical GPIO register memory into our program's address space.
    // This is required by 'rpi_gpio.h' function to interact with the GPIO hardware.
    // The if statement checks for any errors in setting up our pins
    if (!rpi_gpio_map_regs(RPI_PERIPHERAL_BASE)) {
        printf("Failed to map GPIO registers.\n");
        return false;
    }

    // Initialize the LED GPIO pin to be an output
    rpi_gpio_set_select(GPIO_LED_PIN, RPI_GPIO_FUNC_OUT);

    // Initialize the PIR GPIO pin to be an input
    rpi_gpio_set_select(GPIO_PIR_PIN, RPI_GPIO_FUNC_IN);

    // The while loop continuously reads the data from the PIR sensor and turns the LED on if motion is detected
    while(1){
        // 'rpi_gpio_read' reads the value from the PIR sensor when called
        // The return value is 1, if motion is detected and 0 otherwise
        // If the return is 1 we turn on the LED light
        if(rpi_gpio_read(GPIO_PIR_PIN)){
            rpi_gpio_set(GPIO_LED_PIN);     // Turn on the LED light
            printf("Motion Detected!!!\n");
        }
        else{
            rpi_gpio_clear(GPIO_LED_PIN);
            printf("No Motion\n");
        }

        usleep(100000); // Sleep for 100 ms
    }


}