#!/usr/bin/env python3
"""
Copyright (c) 2025, BlackBerry Limited. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

@file sn3218a-rgb-led.py
@brief Example code for driving the SN3218A 18-channel LED driver over I2C using the smbus module.
 
This demo demonstrates:
  - Resetting the SN3218 at startup and on Ctrl-C exit.
  - Enabling all 18 channels.
  - Displaying a test pattern where each of the 6 RGB LEDs shows a distinct color.
  - Continuously cycling through colors (red, green, blue, off).
 
Wiring:
  - SDA: Connect to I2C SDA.
  - SCL: Connect to I2C SCL.
  - 5V : Connect to VDD on SN3218.
  - GND: Connect to GND.
  - (Optional) SDB pin: Pulled high (or driven via a GPIO).
"""
 
import time
import signal
import sys
import smbus
 
# ------------------- I2C and SN3218 Definitions ----------------------
I2C_BUS = 1                # I2C bus number
SN3218_ADDR = 0x54         # SN3218 default I2C address
 
# SN3218 Register Addresses
REG_SHUTDOWN    = 0x00     # Shutdown register: 0 = shutdown; 1 = normal operation
REG_PWM_BASE    = 0x01     # Base address for 18 PWM registers (channels 0..17)
REG_ENABLE_LEDS = 0x13     # LED control registers: 0x13, 0x14, 0x15 (each controls 6 channels)
REG_UPDATE      = 0x16     # Latch register: write any value (e.g., 0x00) to update changes
REG_RESET       = 0x17     # Reset register: write 0xFF to reset chip
 
SN3218_NUM_CHANNELS = 18   # Total channels (6 RGB LEDs x 3 channels each)
 
# ------------------- Channel Mapping ----------------------
# Logical channels map one-to-one to physical channels by default:
# LED0 uses channels 0,1,2; LED1 uses 3,4,5; …; LED5 uses 15,16,17.
channel_map = list(range(SN3218_NUM_CHANNELS))
 
# ------------------- Global Brightness Array ----------------------
# For an active-high design: 255 = full brightness, 0 = off.
g_brightness = [0] * SN3218_NUM_CHANNELS
 
# ------------------- I2C Bus Initialization ----------------------
bus = smbus.SMBus(I2C_BUS)
 
# ------------------- Helper I2C Functions ----------------------
def write_byte(reg, value):
    """
    Writes a single byte to the specified register.
    
    @param reg: Register address (0x00 to 0xFF)
    @param value: 8-bit value to write
    @return: None
    """
    try:
        bus.write_byte_data(SN3218_ADDR, reg, value)
    except Exception as e:
        print(f"ERROR: write_byte(reg=0x{reg:02X}, value=0x{value:02X}) failed: {e}")
 
def write_block(reg, data):
    """
    Writes a block of bytes starting at the specified register.
    This function writes each byte individually, incrementing the register address.
    
    @param reg: Starting register address
    @param data: List of 8-bit values to write
    @return: None
    """
    try:
        for offset, byte in enumerate(data):
            bus.write_byte_data(SN3218_ADDR, reg + offset, byte)
    except Exception as e:
        print(f"ERROR: write_block(reg=0x{reg:02X}, len={len(data)}) failed: {e}")
 
# ------------------- SN3218 Control Functions ----------------------
def sn3218_reset():
    """
    Resets the SN3218 by writing 0xFF to the reset register.
    
    @return: None
    """
    write_byte(REG_RESET, 0xFF)
    time.sleep(0.01)  # 10 ms delay after reset
 
def sn3218_init():
    """
    Initializes the SN3218 by:
      1. Resetting the chip.
      2. Exiting shutdown (writing 0x01 to REG_SHUTDOWN).
      3. Enabling all 18 channels (writing 0x3F to each LED control register).
      4. Clearing the PWM registers.
      5. Latching the changes.
    
    @return: None
    """
    # 1) Reset the chip
    sn3218_reset()
    
    # 2) Exit shutdown
    write_byte(REG_SHUTDOWN, 0x01)
    time.sleep(0.01)
    
    # 3) Enable all 18 channels.
    # Each LED control register (0x13, 0x14, 0x15) controls 6 channels (bits D5:D0).
    # To enable all channels, write 0x3F (binary 00111111) to each register.
    enable_data = [0x3F, 0x3F, 0x3F]
    write_block(REG_ENABLE_LEDS, enable_data)
    time.sleep(0.01)
    
    # 4) Clear the brightness array (set all channels off)
    global g_brightness
    for i in range(SN3218_NUM_CHANNELS):
        g_brightness[i] = 0
    write_block(REG_PWM_BASE, g_brightness)
    
    # 5) Latch the changes by writing to the update register.
    write_byte(REG_UPDATE, 0x00)
    print("SN3218 initialization complete.")
 
def sn3218_update():
    """
    Updates the SN3218 PWM registers with the current brightness values
    and latches the change.
    
    @return: None
    """
    write_block(REG_PWM_BASE, g_brightness)
    write_byte(REG_UPDATE, 0x00)
 
# ------------------- LED Setting Functions ----------------------
def sn3218_set_channel(logical_channel, brightness):
    """
    Sets the brightness for a logical channel using the channel_map.
    
    @param logical_channel: Logical channel index (0 .. SN3218_NUM_CHANNELS-1)
    @param brightness: Brightness value (0..255), where 255 is full brightness (active-high)
    @return: None
    """
    if 0 <= logical_channel < SN3218_NUM_CHANNELS:
        physical_channel = channel_map[logical_channel]
        if physical_channel < SN3218_NUM_CHANNELS:
            g_brightness[physical_channel] = brightness
 
def sn3218_set_rgb(led_index, r, g, b):
    """
    Sets one RGB LED to the specified color.
    
    Logical LED i uses channels (3*i, 3*i+1, 3*i+2).
    
    @param led_index: LED index (0 .. 5)
    @param r: Red brightness (0..255)
    @param g: Green brightness (0..255)
    @param b: Blue brightness (0..255)
    @return: None
    """
    if led_index < 6:
        base = led_index * 3
        sn3218_set_channel(base + 0, r)
        sn3218_set_channel(base + 1, g)
        sn3218_set_channel(base + 2, b)
 
# ------------------- Test Functions ----------------------
def channel_walk_test():
    """
    Runs a channel-walk test: each physical channel (0..17) is turned on one at a time.
    This helps verify the wiring by showing which physical channel drives which LED.
    Each channel is set to full brightness (255) for 1 second.
    
    @return: None
    """
    print("\n--- Channel Walk Test (1 second each) ---")
    for ch in range(SN3218_NUM_CHANNELS):
        for i in range(SN3218_NUM_CHANNELS):
            g_brightness[i] = 0
        g_brightness[ch] = 255
        write_block(REG_PWM_BASE, g_brightness)
        write_byte(REG_UPDATE, 0x00)
        print(f"Physical channel {ch} is ON.")
        time.sleep(1)
    for i in range(SN3218_NUM_CHANNELS):
        g_brightness[i] = 0
    write_block(REG_PWM_BASE, g_brightness)
    write_byte(REG_UPDATE, 0x00)
    print("--- Channel Walk Test Complete ---\n")
 
def sn3218_test_pattern():
    """
    Displays a test pattern by setting each LED (0..5) to a distinct color.
    For an active-high design:
      - LED0: Red (255, 0, 0)
      - LED1: Green (0, 255, 0)
      - LED2: Blue (0, 0, 255)
      - LED3: Yellow (255, 255, 0)
      - LED4: Magenta (255, 0, 255)
      - LED5: Cyan (0, 255, 255)
      
    @return: None
    """
    sn3218_set_rgb(0, 255, 0, 0)      # Red
    sn3218_set_rgb(1, 0, 255, 0)      # Green
    sn3218_set_rgb(2, 0, 0, 255)      # Blue
    sn3218_set_rgb(3, 255, 255, 0)    # Yellow
    sn3218_set_rgb(4, 255, 0, 255)    # Magenta
    sn3218_set_rgb(5, 0, 255, 255)    # Cyan
 
# ------------------- Cleanup & Signal Handler ----------------------
def cleanup_and_exit(signum, frame):
    """
    Cleanup function that turns off all LEDs, resets the SN3218,
    and exits the program. This is invoked upon receiving SIGINT (Ctrl-C).
    
    @param signum: Signal number.
    @param frame: Current stack frame (unused).
    @return: None.
    """
    print(f"\nSignal {signum} caught. Cleaning up...")
    for i in range(SN3218_NUM_CHANNELS):
        g_brightness[i] = 0
    write_block(REG_PWM_BASE, g_brightness)
    write_byte(REG_UPDATE, 0x00)
    sn3218_reset()
    print("SN3218 has been reset. Exiting now.")
    sys.exit(0)
 
# Register the signal handler for Ctrl-C
signal.signal(signal.SIGINT, cleanup_and_exit)
 
# ------------------- Main Function ----------------------
def main():
    print("=== SN3218 LED Driver Demo with Reset & Cleanup ===")
    
    sn3218_init()
    
    # (Optional) Uncomment the next line to run the channel-walk test.
    # channel_walk_test()
    
    sn3218_test_pattern()
    sn3218_update()
    print("Check that LED0..LED5 each show a unique color.")
    time.sleep(3)
    
    while True:
        # All red
        for i in range(6):
            sn3218_set_rgb(i, 255, 0, 0)
        sn3218_update()
        time.sleep(1)
        
        # All green
        for i in range(6):
            sn3218_set_rgb(i, 0, 255, 0)
        sn3218_update()
        time.sleep(1)
        
        # All blue
        for i in range(6):
            sn3218_set_rgb(i, 0, 0, 255)
        sn3218_update()
        time.sleep(1)
        
        # All off
        for i in range(6):
            sn3218_set_rgb(i, 0, 0, 0)
        sn3218_update()
        time.sleep(1)
 
if __name__ == '__main__':
    main()
