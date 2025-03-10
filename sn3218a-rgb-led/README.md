# SN3218A RGB LED Driver for QNX
 
This repository contains sample code (in both C and Python) for interfacing with the SN3218A 18‑channel LED driver on QNX. The driver is designed to control 6 RGB LEDs (3 channels each) via the I²C bus using QNX’s I²C API (via the `rpi_i2c` library). Originally developed for the Trilobot platform, the code can be adapted for use on other hardware setups.
 
> **Important:**  
> The code requires root privileges to access the I²C bus.  
> **Note on I²C Address:** The SN3218A’s default I²C address is **0x54**. Ensure your wiring matches this configuration.
 
## Table of Contents
 
- [Overview](#overview)
- [Hardware Setup](#hardware-setup)
  - [Trilobot/Qwiic Setup](#trilobotqwiic-setup)
  - [Direct Raspberry Pi Connection](#direct-raspberry-pi-connection)
  - [Breadboard/Other Platforms](#breadboardother-platforms)
- [Driver Implementation Details](#driver-implementation-details)
  - [Block Write vs. Single-Byte Write](#block-write-vs-single-byte-write)
- [Software Requirements](#software-requirements)
- [Build and Run Instructions](#build-and-run-instructions)

 
## Overview
 
The SN3218A is an 18‑channel LED driver capable of controlling 6 RGB LEDs. Each LED channel is driven by an 8‑bit PWM register. Additionally, the chip has LED control registers that enable or disable outputs on groups of 6 channels. This sample code demonstrates:
 
1. **Driver Initialization:**  
   - Resetting the chip.
   - Exiting shutdown mode.
   - Enabling all 18 channels.
   - Clearing and updating the PWM registers.
   - Latching the configuration.
 
2. **Test Patterns:**  
   - Running a channel‑walk test to verify the wiring.
   - Displaying a test pattern where each RGB LED shows a distinct color.
   - Continuously cycling through colors (red, green, blue, off).
 
3. **Cleanup:**  
   - A signal handler ensures that on Ctrl‑C (SIGINT) the LED state is cleared and the driver is reset before exiting.
 
## Hardware Setup
 
### Trilobot/Qwiic Setup
 
For the Trilobot platform (or when using a Qwiic cable), wiring is typically as follows:
- **SDA:** Connect to the I²C SDA line.
- **SCL:** Connect to the I²C SCL line.
- **5V:** Connect to VDD on the SN3218A.
- **GND:** Connect to Ground.

### Direct Raspberry Pi Connection

If connecting directly to a Raspberry Pi:
- **SDA:** Connect to GPIO Pin 2 (SDA).
- **SCL:** Connect to GPIO Pin 3 (SCL).
- **5V:** Connect to the Pi’s 5V supply.
- **GND:** Connect to Ground.
 
### Breadboard / Other Platforms
 
For use on a breadboard or custom setup:
- **Power:** Provide a stable 5V (or 3.3V) supply with common ground.
- **I²C:** Connect SDA and SCL to the appropriate bus lines.
- **Address Configuration:**  
  The SN3218A’s I²C address is determined by the wiring. The default is **0x54**.
 
## Driver Implementation Details
 
The driver initializes the SN3218A by writing to several registers:
 
1. **Reset and Shutdown:**  
   The chip is reset (by writing `0xFF` to the reset register) and then taken out of shutdown mode (by writing `0x01` to the shutdown register).
 
2. **Channel Enable:**  
   Three LED control registers (at addresses `0x13`, `0x14`, and `0x15`) enable the outputs.  
   Each register controls 6 channels using bits D5–D0. To enable all 6 channels, each register must be written with `0x3F` (binary `00111111`).
 
3. **PWM Registers and Latching:**  
   The PWM registers (addresses `0x01` through `0x12`) set the brightness for each channel. After updating these registers, a write to the update register (`0x16`) latches the new settings.
 
### Block Write vs. Single-Byte Write
 
Originally, the code used a block write function (e.g. `smbus_write_block_data()`) to update multiple registers at once. However, on our target system the block write function was unreliable—especially for the last LED (channels 15–17), which never lit. By switching to a loop that writes each byte individually (using `smbus_write_byte_data()` for each register address), the auto-increment behavior of the SN3218A is correctly utilized and all channels are updated reliably.
 
This modification resolved the issue where the last LED remained off.
 
## Software Requirements
 
- **QNX Operating System 8.0 or later**
- **I²C API:**  
  The code uses QNX’s I²C API via the `rpi_i2c` library (located in the `common/rpi_i2c` folder).
- **Root Privileges:**  
  Root access is required to access the I²C bus.
 
## Build and Run Instructions
 
### C Code
 
From the `sn3218a-rgb-led/c/` directory, build the code with:
 
```bash
make all
