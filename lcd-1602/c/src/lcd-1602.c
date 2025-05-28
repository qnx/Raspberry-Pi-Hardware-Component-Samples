/*
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
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "rpi_i2c.h"  // I2C API functions for Raspberry Pi or QNX

// I2C address for the PCF8574 I/O expander (controls the LCD)
#define I2C_ADDR 0x27
#define BUS 1  // I2C bus number

// LCD configuration
#define LCD_WIDTH 16        // Characters per line
#define LCD_CHR 1           // Sending data
#define LCD_CMD 0           // Sending command
#define LCD_LINE_1 0x80     // Address for line 1
#define LCD_LINE_2 0xC0     // Address for line 2
#define ENABLE 0b00000100   // Enable bit for toggling

// Backlight control (0x08 = on, 0x00 = off)
uint8_t LCD_BACKLIGHT = 0x00;

// Delay constants (in microseconds)
#define E_PULSE_US 500
#define E_DELAY_US 500

// Function declarations
void lcd_toggle_enable(uint8_t bits);
void lcd_byte(uint8_t bits, uint8_t mode);
void lcd_init();
void lcd_string(const char *message, uint8_t line);
void marquee_smooth(const char *message, uint8_t line, int delay_ms, int cycles);

int main() {
    // Initialize the LCD
    lcd_init();

    // Enable backlight
    LCD_BACKLIGHT = 0x08;

    // Display messages
    lcd_string("Hello, world!", LCD_LINE_1);
    marquee_smooth("C on QNX!", LCD_LINE_2, 300, 3);

    // Wait and then clear the screen
    sleep(5);
    LCD_BACKLIGHT = 0x00;
    lcd_byte(0x01, LCD_CMD);  // Clear display

    // Clean up I2C
    smbus_cleanup(BUS);
    return 0;
}

// Initialize the LCD with standard commands
void lcd_init() {
    lcd_byte(0b00110011, LCD_CMD); // Initialization step 1
    lcd_byte(0b00110010, LCD_CMD); // Initialization step 2
    lcd_byte(0b00000110, LCD_CMD); // Set cursor move direction
    lcd_byte(0b00001100, LCD_CMD); // Display ON, cursor OFF, blink OFF
    lcd_byte(0b00101000, LCD_CMD); // Function set: 2 lines, 5x8 font
    lcd_byte(0b00000001, LCD_CMD); // Clear display
    usleep(E_DELAY_US);            // Short delay
}

// Send byte to LCD (as command or data), split into two 4-bit transfers
void lcd_byte(uint8_t bits, uint8_t mode) {
    uint8_t bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    uint8_t bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    smbus_write_byte_data(BUS, I2C_ADDR, 0, bits_high);
    lcd_toggle_enable(bits_high);

    smbus_write_byte_data(BUS, I2C_ADDR, 0, bits_low);
    lcd_toggle_enable(bits_low);
}

// Toggle the enable bit to latch data into the LCD
void lcd_toggle_enable(uint8_t bits) {
    usleep(E_DELAY_US);
    smbus_write_byte_data(BUS, I2C_ADDR, 0, bits | ENABLE);
    usleep(E_PULSE_US);
    smbus_write_byte_data(BUS, I2C_ADDR, 0, bits & ~ENABLE);
    usleep(E_DELAY_US);
}

// Display a string on a specified LCD line
void lcd_string(const char *message, uint8_t line) {
    char padded[LCD_WIDTH + 1];
    int len = strlen(message);

    // Pad or truncate message to LCD width
    for (int i = 0; i < LCD_WIDTH; i++) {
        if (i < len) {
            padded[i] = message[i];
        } else {
            padded[i] = ' ';
        }
    }
    padded[LCD_WIDTH] = '\0';  // Safe null-termination

    lcd_byte(line, LCD_CMD);  // Set cursor position
    for (int i = 0; i < LCD_WIDTH; i++) {
        lcd_byte(padded[i], LCD_CHR);  // Send each character
    }
}

// Smooth scrolling marquee text across a single LCD line
void marquee_smooth(const char *message, uint8_t line, int delay_ms, int cycles) {
    int msg_len = strlen(message);
    char scroll_text[64];  // Buffer to hold scrollable text
    snprintf(scroll_text, sizeof(scroll_text), "%-*s", msg_len + LCD_WIDTH, message); // Pad with spaces

    for (int c = 0; c < cycles; c++) {
        for (int pos = 0; pos < msg_len + LCD_WIDTH; pos++) {
            char window[LCD_WIDTH + 1];
            for (int i = 0; i < LCD_WIDTH; i++) {
                window[i] = scroll_text[(pos + i) % (msg_len + LCD_WIDTH)];
            }
            window[LCD_WIDTH] = '\0';
            lcd_string(window, line);
            usleep(delay_ms * 1000);  // Delay between shifts
        }
    }
}