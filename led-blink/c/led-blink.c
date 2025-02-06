#include <stdio.h>    // Standard input/output functions (for puts and perror)
#include <stdlib.h>   // Standard library (for EXIT_SUCCESS and EXIT_FAILURE)
 
// The interface for a GPIO resource manager tailored for the Raspberry Pi's GPIO pins under QNX.
// This file provides functions to configure GPIO pins, set pin modes, and read/write pin values.
#include "rpi_gpio.h"
 
// This global variable is used to store a pointer to the Raspberry Pi's GPIO registers.
// It is declared as volatile to prevent compiler optimizations that may interfere with hardware access.
volatile uint32_t *rpi_gpio_regs;
 
// Define the GPIO pin number (PIN 36 on Raspberry Pi 4, which corresponds to GPIO16)
const int data_pin = 16;
 
int main(void) {
 
    // Map the physical GPIO register memory into our program's address space.
    // This is required by 'rpi_gpio.h' to interact with the GPIO hardware.
    rpi_gpio_regs = mmap(0, __PAGESIZE,          // Map one memory page
                         PROT_READ | PROT_WRITE | PROT_NOCACHE,  // Allow read/write, disable cache
                         MAP_SHARED | MAP_PHYS,  // Shared mapping of physical memory
                         -1, 0xfe200000);        // Memory address of GPIO registers
 
    // Check if memory mapping failed
    if (rpi_gpio_regs == MAP_FAILED) {
        perror("GPIO error");  // Print an error message
        return EXIT_FAILURE;   // Exit the program with a failure status
    }
 
    // Set the GPIO pin as an output (needed to control an LED)
    rpi_gpio_set_select(data_pin, RPI_GPIO_FUNC_OUT);
 
    // Turn the LED ON (set the GPIO pin)
    rpi_gpio_set(data_pin);
 
    // Exit successfully
    return EXIT_SUCCESS;
}