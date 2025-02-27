#include <stdio.h>    // Standard input/output functions (for puts and perror)
#include <stdlib.h>   // Standard library (for EXIT_SUCCESS and EXIT_FAILURE)
#include <time.h>

// The interface for a GPIO resource manager tailored for the Raspberry Pi's GPIO pins under QNX.
// This file provides functions to configure GPIO pins, set pin modes, and read/write pin values.
#include "rpi_gpio.h"

// Define the GPIO pin number for each pin
const int red_pin = 17; // GPIO pin for Red LED (pin 11)
const int green_pin = 27; // GPIO pin for Green LED (pin 13)
const int blue_pin = 22; // GPIO pin for Blue LED (pin 15)

// Turns on the LED connected to the specified GPIO pin.
static bool led_on(int gpio_pin)
{
    // Set selected GPIO to high.
    if (rpi_gpio_output(gpio_pin, GPIO_HIGH))
    {
        perror("rpi_gpio_output");
        return false;
    }

    return true;
}

// Turns off the LED connected to the specified GPIO pin.
static bool led_off(int gpio_pin)
{
    // Set selected GPIO to low.
    if (rpi_gpio_output(gpio_pin, GPIO_LOW))
    {
        perror("rpi_gpio_output");
        return false;
    }

    return true;
}

// Turns off all LEDs.
void turnOff() {
	led_off(red_pin);
	led_off(green_pin);
	led_off(blue_pin);
}

// Turns on only the red LED.
void red() {
	led_on(red_pin);
	led_off(green_pin);
	led_off(blue_pin);
}

// Turns on only the green LED.
void green() {
	led_off(red_pin);
	led_on(green_pin);
	led_off(blue_pin);
}

// Turns on only the blue LED.
void blue() {
	led_off(red_pin);
	led_off(green_pin);
	led_on(blue_pin);
}

// Turns on red and green LEDs to produce yellow light.
void yellow() {
	led_on(red_pin);
	led_on(green_pin);
	led_off(blue_pin);
}

// Turns on all LEDs to produce white light.
void white() {
	led_on(red_pin);
	led_on(green_pin);
	led_on(blue_pin);
}

int main(void) {

    // Configure the given GPIO pins as an output pins.
    if (rpi_gpio_setup(red_pin, GPIO_OUT) || rpi_gpio_setup(green_pin, GPIO_OUT) || rpi_gpio_setup(blue_pin, GPIO_OUT))
    {
        perror("rpi_gpio_setup failed");
        return EXIT_FAILURE; // Exit the program with a failure status
    }

    // Infinite loop to cycle through LED colors
    while (1) {
        turnOff();
        delay(1000);

        red();
        delay(1000);

        green();
        delay(1000);

        blue();
        delay(1000);

        yellow();
        delay(1000);

        white();
        delay(1000);
    }

    // Exit successfully (won't ever get here due to endless loop above)
    return EXIT_SUCCESS;
}
