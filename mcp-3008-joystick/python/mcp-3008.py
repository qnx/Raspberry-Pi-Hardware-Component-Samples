# Copyright (c) 2026, BlackBerry Limited. All rights reserved.
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

# In the sample wiring we used SPIO0 with SPIO0_CE0 (GPIO8). If you need to connect two SPI devices
# Use the same wiring for both with the exception of CS which should instead be connected to SPIO0_CE1 (GPIO7)
# and update this value to 1.
MCP3008_CS = 0
MCP3008_START_BIT = 0x01
MCP3008_SGL_DIFF = 0x08  # Single-ended mode
MCP3008_REF_VOLT = 3.3
MCP3008_RESOLUTION = 2**10 - 1

JOYSTICK_X_CHANNEL = 0
JOYSTICK_Y_CHANNEL = 1
JOYSTICK_BUTTON_PIN = 27


def mcp3008_read_channel_raw(channel: int) -> int:
    """
    Reads the raw ADC value from a provided channel.

    @param channel Channel [0-7] to be read
    @return return value in range [0,2^10-1]
    """
    if channel < 0 or channel > 7:
        raise ValueError(f"Invalid mcp3008 channel! Expected [0-7], got {channel}")

    # When during a xfer with SPI we must provide both input and output buffer of the same length.
    data_in = [MCP3008_START_BIT, (MCP3008_SGL_DIFF | channel) << 4, 0]
    data_out = [0] * 3

    # Perform SPI transaction
    GPIO.write_read_spi(MCP3008_CS, data_in, data_out)

    # Extract 10-bit result from returned bytes
    return ((data_out[1] & 0x03) << 8) | data_out[2]


def as_volt(raw_val: int) -> float:
    """
    Calculate the voltage from the ADC based on the reference voltage and ADC's resolution.
    """
    return (raw_val / MCP3008_RESOLUTION) * MCP3008_REF_VOLT


# In order to use SPI we need to initialize it first with the clkdiv, which is 500000000 / clkdiv.
# in our case this is 500000000 / 500 = 1000000 (1 Mhz)
GPIO.init_spi(500)
GPIO.setup(JOYSTICK_BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

while True:
    x = mcp3008_read_channel_raw(JOYSTICK_X_CHANNEL)
    y = mcp3008_read_channel_raw(JOYSTICK_Y_CHANNEL)
    pushed = not GPIO.input(JOYSTICK_BUTTON_PIN)  # Pull up button so 0 means pressed
    print(f"Joystick X: {x:04d}({as_volt(x):.2f}V) Y: {y:04d}({as_volt(y):.2f}V) Pushed:{pushed}")

    # Sleep for 100ms between polling the joystick.
    time.sleep(0.1)
