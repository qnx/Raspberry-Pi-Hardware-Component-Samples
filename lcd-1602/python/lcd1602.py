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

import smbus
import time

# Define the I2C address of the PCF8574 I/O expander connected to the LCD
I2C_ADDR = 0x27

# LCD configuration constants
LCD_WIDTH = 16        # Number of characters per line
LCD_CHR = 1           # Data mode
LCD_CMD = 0           # Command mode
LCD_LINE_1 = 0x80     # Address for the first line
LCD_LINE_2 = 0xC0     # Address for the second line
LCD_BACKLIGHT = 0x00  # Backlight off (set to 0x08 to turn it on)
ENABLE = 0b00000100   # Enable bit to latch data

# Timing delays
E_PULSE = 0.0005      # Enable pulse duration
E_DELAY = 0.0005      # Delay between operations

# Initialize the I2C bus (bus 1 for Raspberry Pi)
bus = smbus.SMBus(1)

def lcd_init():
    """Initializes the LCD with standard 4-bit commands."""
    lcd_byte(0b00110011, LCD_CMD)  # Reset sequence
    lcd_byte(0b00110010, LCD_CMD)  # Set to 4-bit mode
    lcd_byte(0b00000110, LCD_CMD)  # Set cursor move direction (left to right)
    lcd_byte(0b00001100, LCD_CMD)  # Display ON, cursor OFF, blink OFF
    lcd_byte(0b00101000, LCD_CMD)  # Function set: 2 lines, 5x8 font
    lcd_byte(0b00000001, LCD_CMD)  # Clear display
    time.sleep(E_DELAY)

def lcd_byte(bits, mode):
    """Sends a byte to the LCD in 4-bit mode."""
    bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT      # Upper nibble
    bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT  # Lower nibble

    bus.write_byte(I2C_ADDR, bits_high)   # Send high bits
    lcd_toggle_enable(bits_high)

    bus.write_byte(I2C_ADDR, bits_low)    # Send low bits
    lcd_toggle_enable(bits_low)

def lcd_toggle_enable(bits):
    """Pulse the enable bit to latch data into the LCD."""
    time.sleep(E_DELAY)
    bus.write_byte(I2C_ADDR, (bits | ENABLE))   # Enable high
    time.sleep(E_PULSE)
    bus.write_byte(I2C_ADDR, (bits & ~ENABLE))  # Enable low
    time.sleep(E_DELAY)

def lcd_string(message, line):
    """Displays a message on the specified LCD line."""
    message = message.ljust(LCD_WIDTH, " ")  # Pad message with spaces to fill line
    lcd_byte(line, LCD_CMD)                 # Set cursor to the specified line
    for i in range(LCD_WIDTH):
        lcd_byte(ord(message[i]), LCD_CHR)  # Send each character

def marquee_smooth(message, line, delay=0.3, cycles=3):
    """Displays a scrolling marquee effect on the specified LCD line."""
    if len(message) < LCD_WIDTH:
        message = message.ljust(LCD_WIDTH)  # Pad if message is short

    scroll_text = message + " " * LCD_WIDTH  # Add space for smooth scrolling
    length = len(message) + LCD_WIDTH        # Total scroll length

    for cycle in range(cycles):              # Repeat for given number of cycles
        for pos in range(length):
            window = ""                      # Window of characters to show
            for i in range(LCD_WIDTH):
                window += scroll_text[(pos + i) % len(scroll_text)]
            lcd_string(window, line)         # Show scrolling window
            time.sleep(delay)                # Wait between steps

# --- Main Program ---

lcd_init()                  # Initialize the LCD
LCD_BACKLIGHT = 0x08        # Turn on the backlight

lcd_string("Hello, world!", LCD_LINE_1)              # Display a message
marquee_smooth("Python on QNX 8!", LCD_LINE_2)       # Scroll second message

time.sleep(5)                                         # Wait for a while
LCD_BACKLIGHT = 0x00                                 # Turn off backlight
lcd_byte(0x01, LCD_CMD)                              # Clear the display