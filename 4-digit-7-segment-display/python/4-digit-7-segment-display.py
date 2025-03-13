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
import time  # Import the time module for delays

# Define GPIO pins for each segment
segments = {
    "A": 23, "B": 6, "C": 20, "D": 5,
    "E": 24, "F": 19, "G": 12, "DP": 21
}

# Define GPIO pins for digit control (common anode configuration)
digits_pins = [18, 13, 26, 25]

# Segment configurations for displaying digits 0-9 on a 7-segment display
# Each list represents the states of segments [A, B, C, D, E, F, G, DP] (1 = ON, 0 = OFF)
digits = {
    0: [1,1,1,1,1,1,0,0], 
    1: [0,1,1,0,0,0,0,0], 
    2: [1,1,0,1,1,0,1,0], 
    3: [1,1,1,1,0,0,1,0],
    4: [0,1,1,0,0,1,1,0], 
    5: [1,0,1,1,0,1,1,0], 
    6: [1,0,1,1,1,1,1,0], 
    7: [1,1,1,0,0,0,0,0],
    8: [1,1,1,1,1,1,1,0], 
    9: [1,1,1,1,0,1,1,0]
}

# Use Broadcom (BCM) numbering scheme for GPIO pins
GPIO.setmode(GPIO.BCM)

# Set all segment and digit control pins as output
for pin in segments.values():
    GPIO.setup(pin, GPIO.OUT)

for pin in digits_pins:
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, 1)  # Turn all digits OFF initially (common anode HIGH = OFF)

def display_number(num):
    num_str = str(num).zfill(4)  # Convert to 4-digit string, padding with zeros if needed
    
    for i, digit_char in enumerate(num_str):
        digit = int(digit_char)  # Convert character to integer

        GPIO.output(digits_pins[i], 0)  # Turn ON the current digit

        pattern = digits[digit]  # Get segment pattern
        for j, segment in enumerate(segments.values()):
            GPIO.output(segment, pattern[j])  # Set segment states

        time.sleep(0.001)  # Small delay
        GPIO.output(digits_pins[i], 1)  # Turn OFF the digit

try:
    while True:
        for num in range(10000):  # Cycle through 0000-9999
            for _ in range(10):  # Multiplex each number for stability
                display_number(num)

finally:
    GPIO.cleanup()  # Cleanup on exit
