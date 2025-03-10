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

# Define GPIO pins for each segment of the 7-segment display
segments = {
    "A": 19, "B": 26, "C": 25, "D": 20,
    "E": 21, "F": 13, "G": 6, "DP": 12  # DP (Decimal Point)
}

# Segment configurations for digits 0-9 (Common Cathode display)
# 1 = ON, 0 = OFF (Each list represents the segments that should be lit for a given number)
digits = {
    0: [1,1,1,1,1,1,0,0],  # Displays '0'
    1: [0,1,1,0,0,0,0,0],  # Displays '1'
    2: [1,1,0,1,1,0,1,0],  # Displays '2'
    3: [1,1,1,1,0,0,1,0],  # Displays '3'
    4: [0,1,1,0,0,1,1,0],  # Displays '4'
    5: [1,0,1,1,0,1,1,0],  # Displays '5'
    6: [1,0,1,1,1,1,1,0],  # Displays '6'
    7: [1,1,1,0,0,0,0,0],  # Displays '7'
    8: [1,1,1,1,1,1,1,0],  # Displays '8'
    9: [1,1,1,1,0,1,1,1]   # Displays '9' (and DP/Decimal Point)
}

# Set the GPIO mode to BCM (Broadcom SOC channel numbering)
GPIO.setmode(GPIO.BCM)

# Set all segment pins as output
for pin in segments.values():
    GPIO.setup(pin, GPIO.OUT)

# Display a single digit on the 7-segment display
def display_number(num):

    # Get the list of 0s and 1s that determine which segments should be ON/OFF.
    pattern = digits.get(num, [0,0,0,0,0,0,0,0])  # Default to blank if invalid number

    for i, segment in enumerate(segments):  # Loop with an index (i) and segment name (i.e. "A" or "B")
        pin = segments[segment]  # Get the corresponding GPIO pin number for that segment
        state = pattern[i]  # Get the state (ON or OFF) using the index
        GPIO.output(pin, state)  # Set the pin to ON (1) or OFF (0)

try:
    for i in range(10):  # Loop through numbers 0-9
        display_number(i)  # Display the current number
        time.sleep(1)  # Keep the number displayed for 1 second

finally:
    GPIO.cleanup()  # Ensure GPIO cleanup on script exit