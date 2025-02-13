/*
 * Copyright (c) 2024, BlackBerry Limited. All rights reserved.
 *
 * BlackBerry Limited and its licensors  retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from BlackBerry Limited
 * is strictly prohibited.
 */

#ifndef RPI_GPIO_API_H
#define RPI_GPIO_API_H

#include <sys/rpi_gpio.h>

/* Return codes for client API */
#define GPIO_SUCCESS                         0
#define GPIO_ERROR_NOT_CONNECTED            -1
#define GPIO_ERROR_MSG_NOT_SENT             -2
#define GPIO_ERROR_MSG_EVENT_NOT_REGISTERED -3
#define GPIO_ERROR_INPUT_OUT_OF_RANGE       -4

/* GPIO PIN codes */
#define GPIO_COUNT 28

#define GPIO0 0
#define GPIO1 1
#define GPIO2 2
#define GPIO3 3
#define GPIO4 4
#define GPIO5 5
#define GPIO6 6
#define GPIO7 7
#define GPIO8 8
#define GPIO9 9
#define GPIO10 10
#define GPIO11 11
#define GPIO12 12
#define GPIO13 13
#define GPIO14 14
#define GPIO15 15
#define GPIO16 16
#define GPIO17 17
#define GPIO18 18
#define GPIO19 19
#define GPIO20 20
#define GPIO21 21
#define GPIO22 22
#define GPIO23 23
#define GPIO24 24
#define GPIO25 25
#define GPIO26 26
#define GPIO27 27

/* GPIO pin configuration*/
enum gpio_config_t
{
    GPIO_IN,
    GPIO_OUT
};

/* GPIO pin pull direction */
enum gpio_pull_t
{
    GPIO_PUD_OFF,
    GPIO_PUD_UP,
    GPIO_PUD_DOWN
};

/* GPIO pin level */
enum gpio_level_t
{
    GPIO_LOW,
    GPIO_HIGH
};

/* GPIO pin change events */
enum gpio_change_event_t
{
    GPIO_RISING  = 2,
    GPIO_FALLING = 3,
    GPIO_BOTH    = 4,
    GPIO_ALL     = 5
};

/* PWM channel operation mode */
enum pwm_channel_op_mode_t
{
    GPIO_PWM_MODE_PWM = 0,
    GPIO_PWM_MODE_MS  = 1
};

/* SPI chip select selections */
enum spi_chip_select_t
{
    SPI_CS_0 =          0x0,
    SPI_CS_1 =          0x1,
    SPI_CS_2 =          0x2,
    SPI_CS_MANUAL =     0x3,
    SPI_CS_MASK =       0x3
};

/* Select GPIO configuration (input/output) */
int rpi_gpio_setup(int gpio_pin, unsigned configuration);

/* Set pull-up/pull-down */
int rpi_gpio_setup_pull(int gpio_pin, unsigned configuration, unsigned updown);

/* Set up PWM with frequency and range */
int rpi_gpio_setup_pwm(int gpio_pin, unsigned frequency, unsigned mode);

/* Set PWM duty cycle */
int rpi_gpio_set_pwm_duty_cycle(int gpio_pin, float value);

/* Read GPIO configuration (input/output) */
int rpi_gpio_get_setup(int gpio_pin, unsigned *configuration);

/* Turn GPIO PIN on/off */
int rpi_gpio_output(int gpio_pin, unsigned level);

/* Read GPIO PIN level */
int rpi_gpio_get_output(int gpio_pin, unsigned *level);

/* Report on a GPIO event asynchronously */
int rpi_gpio_add_event_detect(int gpio_pin, int coid, unsigned event, unsigned event_id);

#endif /* RPI_GPIO_API_H */
