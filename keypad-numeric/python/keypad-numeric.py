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

import rpi_gpio as GPIO  # QNX Raspberry Pi GPIO library for pin control
import time  # Time module for delays

# Define GPIO pins for rows (outputs) and columns (inputs)
ROW_PINS = [18, 23, 24, 25]  # Rows will be set as outputs
COL_PINS = [12, 16, 20, 21]  # Columns will be set as inputs

# Define the keypad layout (row x column mapping)
KEYS = [
    ['1', '2', '3', 'A'],
    ['4', '5', '6', 'B'],
    ['7', '8', '9', 'C'],
    ['*', '0', '#', 'D']
]

# Initialize GPIO pins
for row in ROW_PINS:
    GPIO.setup(row, GPIO.OUT)  # Set row pins as outputs
    GPIO.output(row, GPIO.HIGH)  # Keep rows HIGH when inactive

for col in COL_PINS:
    GPIO.setup(col, GPIO.IN, GPIO.PUD_UP)  # Set column pins as inputs with pull-up resistors

# Function to scan the keypad for key presses
def scan_keypad():
    for r, row in enumerate(ROW_PINS):
        GPIO.output(row, GPIO.LOW)  # Activate one row at a time
        time.sleep(0.01)  # Short delay to stabilize signals
        
        for c, col in enumerate(COL_PINS):
            if GPIO.input(col) == GPIO.LOW:  # Check if a key is pressed
                print(f"Key Pressed: {KEYS[r][c]}")  # Print the detected key
                
                # Wait for key release to prevent multiple detections
                while GPIO.input(col) == GPIO.LOW:
                    time.sleep(0.1)
        
        GPIO.output(row, GPIO.HIGH)  # Reset row to HIGH before scanning the next row

# Main loop to continuously check for key presses
while True:
    scan_keypad()
    time.sleep(0.2)  # Small delay to prevent excessive CPU usage
