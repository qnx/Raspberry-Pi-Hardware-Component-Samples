# Copyright (c) 2025, BlackBerry Limited. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import rpi_gpio as GPIO  # Import the QNX Raspberry Pi GPIO module for controlling GPIO pins
import time  # Import the time module for adding delays

# Red wire to pin 2 (5V)
# Brown wire to pin 6 (Ground)
# Yellow wire into GPIO 18 (pin 12)

# Reference PINs by GPIO numbers
GPIO.setmode(GPIO.BCM)

GPIO_PIN = 18   # GPIO pin for signal (pin 11)

# Set GPIO pin as an output
GPIO.setup(GPIO_PIN, GPIO.OUT)

# Initialize PWM on the GPIO pin with a frequency of 50Hz (standard for servos)
pwm = GPIO.PWM(GPIO_PIN, 50, mode = GPIO.PWM.MODE_MS)

# How much to step between each angle (in degrees)
DEGREES_STEP = 10

# Duty cycle percentage for servo at 0-degree position
MIN_DUTY_CYCLE = 2.5
# Duty cycle percentage for servo at 180-degree position
MAX_DUTY_CYCLE = 12.5

"""
Sets the servo motor to a specific angle using PWM.

The function converts the given angle (0 to 180 degrees) to a corresponding 
PWM duty cycle and updates the servo position.

:param angle: The desired servo angle (0 to 180 degrees).
"""
def SetAngle(angle):
    # Convert angle to duty cycle
    duty = MIN_DUTY_CYCLE + ((angle * (MAX_DUTY_CYCLE - MIN_DUTY_CYCLE)) / 180.0)

    # Update PWM duty cycle
    pwm.ChangeDutyCycle(duty)


degrees = 0

# Rotate from 0 to 180 degrees
while degrees <= 180:
	SetAngle(degrees)
	degrees += DEGREES_STEP
	time.sleep(.5)

# Rotate from 180 degrees to 0
while degrees >= 0:
	SetAngle(degrees)
	degrees -= DEGREES_STEP
	time.sleep(.5)

pwm.stop()