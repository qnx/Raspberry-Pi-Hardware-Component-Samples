# PCA9685 PWM Servo Controller Hardware Sample

This sample shows how to control multiple servos using the PCA9685 16 channel PWM controller on a Raspberry Pi using QNX.

## Usage
This hardware sample is broken into 2 components: a binary pca9685-servo-controller, and a library `libpca9685` which can be used in your own projects.

The basic usage of pca9685-servo-controller us: `pca9685-servo-controller [options] [pin] on off` where:
- [pin] value (0-15) which controls which servo is controlled. If this isn't provide the command is sent to all servos
- an "on" value (0-4096) which controls where the pwm signal changes from low to high
- an "off" value (0-4096) which controls where the pwm signal changes from high to low
> To learn more about how to use the test app use `pca9685-servo-controller -h`

### SG90 Micro Servo
Based on the data sheet a SG90 micro servo uses a PWM period of 50HZ (20ms) with a pulse width of [1ms, 2ms] where 1.5ms is the center. This means in order to move the servo we need to use on and off values which create pulses between [1ms,2ms]. 

To get the on and off PWM values we need to figure out how long a single tick represents for the PCA9685 given that it is 12-bit PWM controller there are 4096 possible values.

```bash
tick_us = 20000us/4096tick  # 4.88us/tick

# Given the tick_us our range is based on 1000us min and 2000us max
min_pwm = 1000/tick_us # ~200
max_pwm = 2000/tick_us # ~410
mid_pwm = (max_pwm-min_pwm) /2 # ~305
```
> Note some servos have slightly smaller or larger ranges so you may want to play with larger or smaller values to get the best performance out of your servos


#### Example commands
```shell
# Setting specific pins

## Middle (Servo 1)
./pca9685-servo-controller -f 50 0 0 305
## Highest Value (Servo 0)
./pca9685-servo-controller -f 50 0 0 508
## Lowest Value (Servo 1)
./pca9685-servo-controller -f 50 1 0 100

# Setting all pins to idle
./pca9685-servo-controller -f 50 0 0

# For more options
./pca9685-servo-controller -h
```

## Pin Configuration

PCA9685 as follows (where NC means not connected):

- PCA9685 GND to GND (pin 6)
- PCA9685 OE NC
- PCA9685 SCL to I2C1 SCL (pin 3)
- PCA9685 SCA to I2C1 SCA (pin 2)
- PCA9685 VCC to 3.3v (pin 1)
- PCA9685 V+ NC

Power wiring (Green Power Terminal)
> NOTE: Do NOT power it directly off of the Raspberry Pi
- PCA9685 V+ to 6v external power connector positive terminal
- PCA9685 GND to 6v external power connector negative terminal

## Schematic Diagrams

<img src="./circuit-pca9685.png" width="50%" />