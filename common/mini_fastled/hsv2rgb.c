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

#include <stdint.h>
#include <math.h>
#include "mini_fastled.h"

// HSV functions
#define APPLY_DIMMING(X) (X)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MIN3(a, b, c) MIN((a), MIN((b), (c)))
#define MAX3(a, b, c) MAX((a), MAX((b), (c)))

#define FIXFRAC8(N, D) (((N) * 256) / (D))

ws2811_led_t hsv_to_led(uint8_t hue, uint8_t saturation, uint8_t value)
{
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t int_value = APPLY_DIMMING(value);
    uint8_t int_saturation = saturation;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = APPLY_DIMMING(255 - int_saturation);
    uint8_t brightness_floor = (int_value * invsat) / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = int_value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are within that section
    uint8_t section = hue / HSV_SECTION_6; // 0..5
    uint8_t offset = hue % HSV_SECTION_6;  // 0..42

    uint8_t rampup = offset;                         // 0..43
    uint8_t rampdown = (HSV_SECTION_6 - 1) - offset; // 43..0

    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj = (rampup * color_amplitude) / HSV_SECTION_6;
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / HSV_SECTION_6;

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor = rampup_amp_adj + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;

    uint8_t brightness_ceiling = 255 + brightness_floor;

    ws2811_led_t led_color = 0x0;

    if (section)
    {
        if (section == 1)
        {
            // section 1: yellow to green
            led_color = ((int)rampdown_adj_with_floor << LED_SHIFT_R) +
                         ((int)brightness_ceiling << LED_SHIFT_G) +
                         ((int)brightness_floor << LED_SHIFT_B);
        }
        else if (section == 2)
        {
            // section 2: green to cyan / aqua
            led_color = ((int)brightness_floor << LED_SHIFT_R) +
                         ((int)brightness_ceiling << LED_SHIFT_G) +
                         ((int)rampup_adj_with_floor << LED_SHIFT_B);
        }
        else if (section == 3)
        {
            // section 3: aqua to blue
            led_color = ((int)brightness_floor << LED_SHIFT_R) +
                         ((int)rampdown_adj_with_floor << LED_SHIFT_G) +
                         ((int)brightness_ceiling << LED_SHIFT_B);
        }
        else if (section == 4)
        {
            // section 4: blue to violet / indigo
            led_color = ((int)rampup_adj_with_floor << LED_SHIFT_R) +
                         ((int)brightness_floor << LED_SHIFT_G) +
                         ((int)brightness_ceiling << LED_SHIFT_B);
        }
        else if (section == 5)
        {
            // section 5: violet / indigo to red
            led_color = ((int)brightness_ceiling << LED_SHIFT_R) +
                         ((int)brightness_floor << LED_SHIFT_G) +
                         ((int)rampdown_adj_with_floor << LED_SHIFT_B);
        }
    }
    else
    {
        // section 0: red to yellow
        led_color = (brightness_ceiling << LED_SHIFT_R) +
                     (rampup_adj_with_floor << LED_SHIFT_G) +
                     (brightness_floor << LED_SHIFT_B);
    }

    return led_color;
}
