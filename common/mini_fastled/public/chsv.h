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

#ifndef __INC_CHSV_H
#define __INC_CHSV_H

#include "rpi_ws281x.h"

// macros for HSV color conversion
#define CHSV(hue,saturation,value) hsv_to_led(hue,saturation,value)

// subdividing hue into six sections
#define HSV_SECTION_6 43

/// Pre-defined hue values for HSV predefined colors, based on six segment color space
typedef enum {
    HUE_RED = 0,
    HUE_ORANGE = (HSV_SECTION_6 / 2),
    HUE_YELLOW = HSV_SECTION_6,
    HUE_GREEN = (HSV_SECTION_6 * 2),
    HUE_AQUA = (HSV_SECTION_6 * 3),
    HUE_BLUE = (HSV_SECTION_6 * 4),
    HUE_PURPLE = (HSV_SECTION_6 * 5),
    HUE_PINK = (int)(HSV_SECTION_6 * 5.5)
} HSVHue;

extern ws2811_led_t hsv_to_led(uint8_t hue, uint8_t saturation, uint8_t value);

#endif