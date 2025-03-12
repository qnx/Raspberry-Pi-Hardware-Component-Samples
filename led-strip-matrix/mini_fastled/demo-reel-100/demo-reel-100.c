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

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#define DATA_PIN LED_CHANNEL_0_DATA_PIN // pin 10
#define LED_TYPE WS2811_STRIP_GRB
#define NUM_LEDS 256

CRGBArray(NUM_LEDS, leds);

#define BRIGHTNESS 96
#define FRAMES_PER_SECOND 120

void setup()
{
    FastLED_delay(3000); // 3 second delay for recovery

    // tell FastLED about the LED strip configuration
    FastLED_addLeds(LED_TYPE, DATA_PIN, &leds, NUM_LEDS);

    FastLED_setCorrection(TypicalLEDStrip);

    // set master brightness control
    FastLED_setBrightness(BRIGHTNESS);
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns
typedef void (*SimplePatternList[])();

void addGlitter(fract8 chanceOfGlitter)
{
    if (random8() < chanceOfGlitter)
    {
        *(leds.get(&leds, random16_lim(NUM_LEDS))) = White;
    }
}

void rainbow()
{
    // FastLED's built-in rainbow generator
    leds.fill_rainbow(&leds, gHue, 7);
}

void rainbowWithGlitter()
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

void confetti()
{
    // random colored speckles that blink in and fade smoothly
    leds.fadeToBlackBy(&leds, 10);
    int pos = random16_lim(NUM_LEDS);
    *(leds.get(&leds, pos)) = CHSV(gHue + random8_lim(64), 200, 255);
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    leds.fadeToBlackBy(&leds, 20);
    int pos = beatsin16(13, 0, NUM_LEDS - 1);
    *(leds.get(&leds, pos)) = CHSV(gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette;
    CRGBPalette16_init(palette, PartyColors_p);
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < NUM_LEDS; i++)
    { // 9948
        *(leds.get(&leds, i)) = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    leds.fadeToBlackBy(&leds, 20);
    uint8_t dothue = 0;
    for (int i = 0; i < 8; i++)
    {
        *(leds.get(&leds, beatsin16(i + 7, 0, NUM_LEDS - 1))) = CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

// List of patterns to cycle through.  Each is defined as a separate function below.
SimplePatternList gPatterns = {rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm};

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void loop()
{
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();

    // send the 'leds' array out to the actual LED strip
    FastLED_show();
    // insert a delay to keep the framerate modest
    FastLED_delay(1000 / FRAMES_PER_SECOND);

    // do some periodic updates
    EVERY_N_MILLISECONDS(20, { gHue++; });   // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS(5, { nextPattern(); }); // change patterns periodically
}