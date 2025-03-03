# librpi_gpio

This folder contains a client API for interacting with
the GPIO resource manager.  The API was designed to be
similar and as easy to use as the GPIO Python module.

## APIs

### rpi_gpio_setup

Select GPIO configuration (input/output)

### rpi_gpio_setup_pull

Set pull-up/pull-down

### rpi_gpio_setup_pwm

Set up PWM with frequency and range

### rpi_gpio_set_pwm_duty_cycle

Set PWM duty cycle

### rpi_gpio_get_setup

Read GPIO configuration (input/output)

### rpi_gpio_output

Turn GPIO PIN on/off

### rpi_gpio_input

Read GPIO PIN level

### rpi_gpio_add_event_detect

Report on a GPIO event asynchronously

### rpi_gpio_cleanup

Cleanup GPIO API resources

---
See [rpi_gpio.h](public/rpi_gpio.h) for more details.
