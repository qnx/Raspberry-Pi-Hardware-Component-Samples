/**
* Copyright (c) 2025, BlackBerry Limited. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* @file sn3218a-rgb-led.c
* @brief Example code for driving the SN3218A 18-channel LED driver over I2C on QNX.
*
* This demo demonstrates:
*   - Resetting the SN3218 at startup and on Ctrl-C exit.
*   - Enabling all 18 channels.
*   - Running a channel-walk test to verify physical channel wiring.
*   - Displaying a test pattern where each of the 6 RGB LEDs shows a distinct color.
*   - Continuously cycling through colors (red, green, blue, off).
*
* Wiring:
*   - SDA -> I2C SDA
*   - SCL -> I2C SCL
*   - 5V  -> VDD on SN3218
*   - GND -> GND
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include "rpi_i2c.h"
 
// ------------------- I2C and SN3218 Definitions ----------------------
#define I2C_BUS            1       // I2C bus number
#define SN3218_ADDR        0x54    // SN3218 default I2C address
 
// SN3218 Register Addresses
#define REG_SHUTDOWN       0x00    // Shutdown register: 0 = shutdown; 1 = normal operation
#define REG_PWM_BASE       0x01    // Base address for 18 PWM registers (channels 0..17)
#define REG_ENABLE_LEDS    0x13    // LED control registers: 0x13, 0x14, 0x15 (each controls 6 channels)
#define REG_UPDATE         0x16    // Latch register: write any value (e.g., 0x00) to update
#define REG_RESET          0x17    // Reset register: write 0xFF to reset chip
 
#define SN3218_NUM_CHANNELS 18     // Total channels (6 RGB LEDs x 3 channels each)
 
// ------------------- Channel Mapping ----------------------
// By default, LED0 uses channels 0,1,2; LED1 uses 3,4,5; …; LED5 uses 15,16,17.
static const uint8_t channel_map[SN3218_NUM_CHANNELS] = {
    0,  1,  2,    // LED0
    3,  4,  5,    // LED1
    6,  7,  8,    // LED2
    9,  10, 11,    // LED3
    12, 13, 14,     // LED4
    15, 16, 17      // LED5 
};
 
// Global brightness array for each physical channel.
// For an active-high design, 255 = full brightness (on) and 0 = off.
static uint8_t g_brightness[SN3218_NUM_CHANNELS] = {0};
 
// ------------------- Forward Declarations ----------------------------
static void cleanup_and_exit(int signum);
 
// ------------------- SMBus Write Helper Functions -----------------------------
/**
* @brief Writes a single byte to the specified register.
*/
static int sn3218_write_byte(uint8_t reg, uint8_t value)
{
    int rc = smbus_write_byte_data(I2C_BUS, SN3218_ADDR, reg, value);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "ERROR: smbus_write_byte_data(reg=0x%02X, val=0x%02X) rc=%d\n", reg, value, rc);
    }
    return rc;
}
 
/**
* @brief Writes multiple bytes starting at the given register by writing them one-by-one.
*
* This function loops over the data array and writes each byte individually,
* incrementing the register address. 
*/
static int sn3218_write_block_loop(uint8_t start_reg, const uint8_t *data, uint8_t length)
{
    int rc = I2C_SUCCESS;
    for (uint8_t i = 0; i < length; i++) {
        rc = smbus_write_byte_data(I2C_BUS, SN3218_ADDR, start_reg + i, data[i]);
        if (rc != I2C_SUCCESS) {
            fprintf(stderr, "ERROR: Failed to write reg 0x%02X\n", start_reg + i);
            return rc;
        }
        usleep(1000); // 1 ms delay between writes
    }
    return rc;
}
 
// ------------------- SN3218 Control Functions -------------------------
/**
* @brief Resets the SN3218 by writing 0xFF to the reset register.
*/
static int sn3218_reset(void)
{
    int rc = sn3218_write_byte(REG_RESET, 0xFF);
    usleep(10000); // 10 ms delay after reset
    return rc;
}
 
/**
* @brief Initializes the SN3218 by resetting the chip, exiting shutdown mode,
*        enabling all 18 channels, clearing brightness values, and latching changes.
*/
static int sn3218_init(void)
{
    int rc;
    
    // 1) Reset the chip
    rc = sn3218_reset();
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "Error resetting SN3218.\n");
        return rc;
    }
    
    // 2) Exit shutdown (write 0x01 to REG_SHUTDOWN)
    rc = sn3218_write_byte(REG_SHUTDOWN, 0x01);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "Error enabling SN3218 output.\n");
        return rc;
    }
    usleep(10000);
    
    // 3) Enable all 18 channels.
    // Each LED control register (0x13, 0x14, 0x15) controls 6 channels (bits D5:D0).
    // To enable each channel, write 0x3F (binary 00111111) to each register.
    {
        uint8_t enable_data[3] = {0x3F, 0x3F, 0x3F};
        rc = sn3218_write_block_loop(REG_ENABLE_LEDS, enable_data, 3);
        if (rc != I2C_SUCCESS) {
            fprintf(stderr, "Error enabling SN3218 channels.\n");
            return rc;
        }
    }
    usleep(10000);
    
    // 4) Clear brightness array (set all channels off)
    for (int i = 0; i < SN3218_NUM_CHANNELS; i++) {
        g_brightness[i] = 0;
    }
    rc = sn3218_write_block_loop(REG_PWM_BASE, g_brightness, SN3218_NUM_CHANNELS);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "Error writing initial brightness.\n");
        return rc;
    }
    
    // 5) Latch changes by writing to REG_UPDATE.
    rc = sn3218_write_byte(REG_UPDATE, 0x00);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "Error updating SN3218.\n");
        return rc;
    }
    
    printf("SN3218 initialization complete.\n");
    return I2C_SUCCESS;
}
 
/**
* @brief Updates the SN3218 PWM registers with the current brightness values
*        and latches the change.
*/
static int sn3218_update(void)
{
    int rc = sn3218_write_block_loop(REG_PWM_BASE, g_brightness, SN3218_NUM_CHANNELS);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "Error writing brightness data.\n");
        return rc;
    }
    rc = sn3218_write_byte(REG_UPDATE, 0x00);
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "Error updating SN3218.\n");
    }
    return rc;
}
 
// ------------------- LED Setting Functions ----------------------------
/**
* @brief Sets the brightness for a logical channel using the channel_map.
*
* @param channel Logical channel index (0 .. SN3218_NUM_CHANNELS-1)
* @param brightness Brightness value (0..255) (for active-high, 255 is on)
*/
static void sn3218_set_channel(uint8_t channel, uint8_t brightness)
{
    if (channel < SN3218_NUM_CHANNELS) {
        uint8_t physical_ch = channel_map[channel];
        if (physical_ch < SN3218_NUM_CHANNELS) {
            g_brightness[physical_ch] = brightness;
        }
    }
}
 
/**
* @brief Sets one RGB LED (of 6) to the specified (R, G, B) color.
*
* Logical LED i uses channels (3*i, 3*i+1, 3*i+2).
*
* @param led_index LED index (0..5)
* @param r Red brightness (0..255)
* @param g Green brightness (0..255)
* @param b Blue brightness (0..255)
*/
static void sn3218_set_rgb(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b)
{
    if (led_index < 6) {
        uint8_t base = led_index * 3;
        sn3218_set_channel(base + 0, r);
        sn3218_set_channel(base + 1, g);
        sn3218_set_channel(base + 2, b);
    }
}
 
// ------------------- Test Functions -------------------------------------
/**
* @brief Runs a channel-walk test that turns on each physical channel (0..17)
*        one by one for 1 second so you can verify the wiring.
*/
static void channel_walk_test(void)
{
    printf("\n--- Channel Walk Test (1 second each) ---\n");
    for (int ch = 0; ch < SN3218_NUM_CHANNELS; ch++) {
        for (int i = 0; i < SN3218_NUM_CHANNELS; i++) {
            g_brightness[i] = 0;
        }
        g_brightness[ch] = 255;
        sn3218_write_block_loop(REG_PWM_BASE, g_brightness, SN3218_NUM_CHANNELS);
        sn3218_write_byte(REG_UPDATE, 0x00);
        printf("  Physical channel %d is now ON.\n", ch);
        sleep(1);
    }
    for (int i = 0; i < SN3218_NUM_CHANNELS; i++) {
        g_brightness[i] = 0;
    }
    sn3218_write_block_loop(REG_PWM_BASE, g_brightness, SN3218_NUM_CHANNELS);
    sn3218_write_byte(REG_UPDATE, 0x00);
    printf("--- Channel Walk Test Complete ---\n\n");
}
 
/**
* @brief Displays a test pattern where each LED is set to a distinct color.
*
* For an active-high design:
*   - LED0: Red (255, 0, 0)
*   - LED1: Green (0, 255, 0)
*   - LED2: Blue (0, 0, 255)
*   - LED3: Yellow (255, 255, 0)
*   - LED4: Magenta (255, 0, 255)
*   - LED5: Cyan (0, 255, 255)
*/
static void sn3218_test_pattern(void)
{
    sn3218_set_rgb(0, 255, 0, 0);      // Red
    sn3218_set_rgb(1, 0, 255, 0);      // Green
    sn3218_set_rgb(2, 0, 0, 255);      // Blue
    sn3218_set_rgb(3, 255, 255, 0);    // Yellow
    sn3218_set_rgb(4, 255, 0, 255);    // Magenta
    sn3218_set_rgb(5, 0, 255, 255);    // Cyan
}
 
// ------------------- Cleanup & Signal Handling ------------------------
/**
* @brief Cleanup function that turns off all LEDs, resets the SN3218,
*        and then exits. Invoked on SIGINT (Ctrl-C).
*/
static void cleanup_and_exit(int signum)
{
    printf("\nSignal %d caught. Cleaning up...\n", signum);
    for (int i = 0; i < SN3218_NUM_CHANNELS; i++) {
        g_brightness[i] = 0;
    }
    sn3218_write_block_loop(REG_PWM_BASE, g_brightness, SN3218_NUM_CHANNELS);
    sn3218_write_byte(REG_UPDATE, 0x00);
    sn3218_reset();
    printf("SN3218 has been reset. Exiting now.\n");
    exit(EXIT_SUCCESS);
}
// ------------------- MAIN --------------------------------------------
int main(void)
{
    // Install signal handler for Ctrl-C
    signal(SIGINT, cleanup_and_exit);
    
    printf("=== SN3218 LED Driver Demo ===\n");
    
    // 1) Initialize the SN3218
    int rc = sn3218_init();
    if (rc != I2C_SUCCESS) {
        fprintf(stderr, "SN3218 init failed (err %d)\n", rc);
        return EXIT_FAILURE;
    }
    
    // 2) (Optional) Run a channel-walk test to verify wiring.
    // Uncomment the next line to run the test.
    // channel_walk_test();
    
    // 3) Display a test pattern (each LED a distinct color) for 3 seconds.
    sn3218_test_pattern();
    rc = sn3218_update();
    if (rc != I2C_SUCCESS) {
        return EXIT_FAILURE;
    }
    printf("Check that LED0..LED5 each show a unique color.\n");
    sleep(3);
    
    // 4) Continuous color cycle: red -> green -> blue -> off
    while (1) {
        // All red
        for (int i = 0; i < 6; i++) {
            sn3218_set_rgb(i, 255, 0, 0);
        }
        sn3218_update();
        sleep(1);
        
        // All green
        for (int i = 0; i < 6; i++) {
            sn3218_set_rgb(i, 0, 255, 0);
        }
        sn3218_update();
        sleep(1);
        
        // All blue
        for (int i = 0; i < 6; i++) {
            sn3218_set_rgb(i, 0, 0, 255);
        }
        sn3218_update();
        sleep(1);
        
        // All off
        for (int i = 0; i < 6; i++) {
            sn3218_set_rgb(i, 0, 0, 0);
        }
        sn3218_update();
        sleep(1);
    }
    
    cleanup_and_exit(0);
    return EXIT_SUCCESS;
}
