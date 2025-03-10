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

// adapted from the FastLED library (https://fastled.io/)

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include "mini_fastled.h"
#include "power_mgt.h"

// base LED set for rpi_ws281x library integration
ws2811_t ledset =
    {
        .freq = WS2811_TARGET_FREQ,
        .channel =
            {
                [0] =
                    {
                        .gpionum = -1,
                        .invert = 0,
                        .count = 0,
                        .leds = NULL,
                        .brightness = 255,
                        .color_correction = 0,
                        .color_temperature = 0,
                        .gamma_factor = 0.0,
                        .gamma = NULL,
                    },
                [1] =
                    {
                        .gpionum = -1,
                        .invert = 0,
                        .count = 0,
                        .leds = NULL,
                        .brightness = 255,
                        .color_correction = 0,
                        .color_temperature = 0,
                        .gamma_factor = 0.0,
                        .gamma = NULL,
                    },
            },
};

// settings backing main FASTLED functions
static uint32_t lastshow = 0; // time of last show call

/// Typedef for a power consumption calculation function. Used within
/// CFastLED for rescaling brightness before sending the LED data to
/// the strip with CFastLED::show().
/// @param scale the initial brightness scale value
/// @param data max power data, in milliwatts
/// @returns the brightness scale, limited to max power
typedef uint8_t (*power_func)(ws2811_t* ledset, uint8_t scale, uint32_t data);

/// High level controller interface for FastLED.
/// This class manages controllers, global settings, and trackings such as brightness
/// and refresh rates, and provides access functions for driving led data to controllers
/// via the show() / showColor() / clear() methods.
/// This is instantiated as a global object with the name FastLED.

static uint16_t m_nFPS;         ///< tracking for current frames per second (FPS) value
static uint32_t m_nMinMicros;   ///< minimum µs between frames, used for capping frame rates
static uint32_t m_nPowerData;   ///< max power use parameter
static power_func m_pPowerFunc; ///< function for overriding brightness when using FastLED.show();

// functions backing main FASTLED functions
/* The `FastLED_addLeds` function is a custom function that is used to add LED strips to the FastLED
library. It takes in parameters such as the type of LED strip, the pin number to which the strip is
connected, a pointer to the LED data, and the number of LEDs in the strip. */
void FastLED_addLeds(unsigned strip_type, unsigned strip_pin, rgb_pixel_set *rps, unsigned number_leds)
{
    // initialize common settings for all LED channels
    m_nFPS = 0;
	m_pPowerFunc = NULL;
	m_nPowerData = 0xFFFFFFFF;
	m_nMinMicros = 0;

    if (ledset.channel[0].gpionum == -1)
    {
        ledset.channel[0].gpionum = strip_pin;
        ledset.channel[0].strip_type = strip_type;
        ledset.channel[0].count = number_leds;
        if (rps->leds == NULL)
        {
            rps->length = number_leds;
            rps->leds = malloc(rps->length * sizeof(ws2811_led_t));
            memset(rps->leds, 0, rps->length * sizeof(ws2811_led_t));
        }
        ledset.channel[0].leds = rps->leds;
        ledset.channel[0].brightness = 255;
    }
    else if (ledset.channel[1].gpionum == -1)
    {
        ledset.channel[1].gpionum = strip_pin;
        ledset.channel[1].strip_type = strip_type;
        ledset.channel[1].count = number_leds;
        if (rps->leds == NULL)
        {
            rps->length = number_leds;
            rps->leds = malloc(rps->length * sizeof(ws2811_led_t));
            memset(rps->leds, 0, rps->length * sizeof(ws2811_led_t));
        }
        ledset.channel[1].leds = rps->leds;
        ledset.channel[1].brightness = 255;
    }
    else
    {
        // skip as there are only two channels supported
    }

    return;
}

uint8_t FastLED_getBrightness()
{
    return ledset.channel[0].brightness;
}

void FastLED_setBrightness(uint8_t brightness)
{
    ledset.channel[0].brightness = brightness;
    ledset.channel[1].brightness = brightness;
}

void FastLED_setMaxPowerInVoltsAndMilliamps(uint8_t volts, uint32_t milliamps)
{
    FastLED_setMaxPowerInMilliWatts(volts * milliamps);
}

void FastLED_setMaxPowerInMilliWatts(uint32_t milliwatts)
{
    m_pPowerFunc = &calculate_max_brightness_for_power_mW_ledset;
    m_nPowerData = milliwatts;
}

void FastLED_setCorrection(LEDColorCorrection color_correction)
{
    ws2811_set_color_correction(&ledset, color_correction);
}

void FastLED_setTemperature(ws2811_led_t color_temperature)
{
    ws2811_set_color_temperature(&ledset, color_temperature);
}

void FastLED_showAt(uint8_t brightness)
{
    while(m_nMinMicros && ((micros()-lastshow) < m_nMinMicros));
	lastshow = micros();

	// If we have a function for computing power, use it!
	if(m_pPowerFunc) {
		brightness = (*m_pPowerFunc)(&ledset, brightness, m_nPowerData);
	}

    FastLED_setBrightness(brightness);
    ws2811_render(&ledset);
}

// Clear the matrix from start s to end e
void FastLED_clear(bool write_data)
{
    if (ledset.channel[0].gpionum != -1)
    {
        if (ledset.channel[0].leds)
        {
            for (int index = 0; index < ledset.channel[0].count; index++)
            {
                ledset.channel[0].leds[index] = 0;
            }
        }
    }
    if (ledset.channel[1].gpionum != -1)
    {
        if (ledset.channel[1].leds)
        {
            for (int index = 0; index < ledset.channel[1].count; index++)
            {
                ledset.channel[1].leds[index] = 0;
            }
        }
    }

    if (write_data)
    {
        FastLED_show();
    }
}

// external references to functions that should be defined to interoperate with mini_fastled

// user function to include extra setup code
extern void setup();

// user function that serves as main loop
extern void loop();

static bool clear_on_exit = true;
static bool running = true;

static void ctrl_c_handler(int signum)
{
    (void)(signum);
    running = 0;
}

static void setup_handlers(void)
{
    struct sigaction sa =
        {
            .sa_handler = ctrl_c_handler,
        };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static void setup_all(void)
{
    ws2811_return_t ret;

    setup_handlers();

    setup();

    if ((ret = ws2811_init(&ledset)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        exit(ret);
    }
}

int main(int argc, char *argv[])
{
    setup_all();

    while (running)
    {
        loop();
    }

    if (clear_on_exit)
    {
        FastLED_clear(true);
    }

    ws2811_fini(&ledset);

    printf("\nExiting ...\n");

    exit(0);
}
