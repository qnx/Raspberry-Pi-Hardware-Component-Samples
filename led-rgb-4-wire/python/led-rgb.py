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
 
# Define GPIO pins for RGB LED
RED_PIN = 17   # GPIO pin for Red LED (pin 11)
GREEN_PIN = 27 # GPIO pin for Green LED (pin 13)
BLUE_PIN = 22  # GPIO pin for Blue LED (pin 15)
 
# Set up GPIO pins as output
GPIO.setup(RED_PIN, GPIO.OUT)
GPIO.setup(GREEN_PIN, GPIO.OUT)
GPIO.setup(BLUE_PIN, GPIO.OUT)
 

# Turns off all LEDs.
def turnOff():
    GPIO.output(RED_PIN, GPIO.LOW)
    GPIO.output(GREEN_PIN, GPIO.LOW)
    GPIO.output(BLUE_PIN, GPIO.LOW)
          
# Turns on only the red LED.
def red():
    GPIO.output(RED_PIN, GPIO.HIGH)
    GPIO.output(GREEN_PIN, GPIO.LOW)
    GPIO.output(BLUE_PIN, GPIO.LOW)

# Turns on only the green LED. 
def green():
    GPIO.output(RED_PIN, GPIO.LOW)
    GPIO.output(GREEN_PIN, GPIO.HIGH)
    GPIO.output(BLUE_PIN, GPIO.LOW)
     
# Turns on only the blue LED.
def blue():
    GPIO.output(RED_PIN, GPIO.LOW)
    GPIO.output(GREEN_PIN, GPIO.LOW)
    GPIO.output(BLUE_PIN, GPIO.HIGH)

# Turns on red and green LEDs to produce yellow light.    
def yellow():
    GPIO.output(RED_PIN, GPIO.HIGH)
    GPIO.output(GREEN_PIN, GPIO.HIGH)
    GPIO.output(BLUE_PIN, GPIO.LOW)

# Turns on all LEDs to produce white light.
def white():
    GPIO.output(RED_PIN, GPIO.HIGH)
    GPIO.output(GREEN_PIN, GPIO.HIGH)
    GPIO.output(BLUE_PIN, GPIO.HIGH)

# Infinite loop to cycle through LED colors
while True:
 
    # Show each colour defined above with a 1 second delay between each one
 
    turnOff()
    time.sleep(1)
 
    red()
    time.sleep(1)
 
    green()
    time.sleep(1)
 
    blue()
    time.sleep(1)
 
    yellow()
    time.sleep(1)
  
    white()
    time.sleep(1)