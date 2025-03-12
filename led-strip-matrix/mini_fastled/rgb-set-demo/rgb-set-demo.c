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
#include <sys/neutrino.h>
#include "mini_fastled.h"

#define LED_PIN LED_CHANNEL_0_DATA_PIN // pin 10

#define NUM_LEDS 256

CRGBArray(NUM_LEDS, leds);

void setup() { FastLED_addLeds(NEOPIXEL, LED_PIN, &leds, NUM_LEDS); }

void loop()
{
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_LEDS / 2; i++)
  {
    // fade everything out
    leds.fadeToBlackBy(&leds, 40);

    // increment hue every 4 pixels ...
    if ((i % 4) == 3)
    {
      hue++;
    }

    // let's set an led value
    *(leds.get(&leds, i)) = CHSV(hue, 255, 255);

    // now, let's set the colors of the first 20 leds to the colors of the top 20 leds,
    leds.copyFrom(&leds, NUM_LEDS / 2, NUM_LEDS - 1, &leds, NUM_LEDS / 2 - 1, 0);

    FastLED_show();

    FastLED_delay(33);
  }
}
