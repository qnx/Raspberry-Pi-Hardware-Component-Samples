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
 
# Set GPIO pin 16 as an output pin
GPIO.setup(16, GPIO.OUT)
 
# Initially set GPIO pin 16 to LOW (turning it OFF)
GPIO.output(16, GPIO.LOW)
 
# Infinite loop to blink the LED
while True:
    GPIO.output(16, GPIO.HIGH)  # Turn LED ON
    time.sleep(0.5)  # Wait for 0.5 seconds
 
    GPIO.output(16, GPIO.LOW)  # Turn LED OFF
    time.sleep(0.5)  # Wait for 0.5 seconds