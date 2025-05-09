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

import rpi_gpio as GPIO
import time

# Rotary encoder pin definitions
CLK = 17
DT = 18
SW = 27

# Set up pins
GPIO.setup(CLK, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(DT, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(SW, GPIO.IN, GPIO.PUD_UP)

# Track the previous state
last_clk_state = GPIO.input(CLK)
# Start at position 0
position = 0

# Handle button press
def check_button(pin):
    if GPIO.input(SW) == GPIO.LOW:
        print("Button Pressed")

# Detect button press
GPIO.add_event_detect(SW, GPIO.FALLING, callback=check_button)

try:
    while True:
        clk_state = GPIO.input(CLK)
        dt_state = GPIO.input(DT)

        # Only act on falling edge of CLK
        if clk_state == GPIO.LOW and last_clk_state == GPIO.HIGH:
            if dt_state == GPIO.HIGH:
                position -= 1
                print("Rotated to", position)
            else:
                position += 1
                print("Rotated to", position)

        last_clk_state = clk_state
        time.sleep(0.001)  # Small delay to debounce

# Exit gracefully when Ctrl+C is pressed
except KeyboardInterrupt:
    print("Exiting...")

finally:
    GPIO.cleanup()