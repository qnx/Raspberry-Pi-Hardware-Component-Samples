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

#define LED_PIN 10

// Information about the LED strip itself

#define NUM_LEDS 256
#define STRIP_TYPE WS2811_STRIP_GRB

#define BRIGHTNESS 128

CRGBArray(NUM_LEDS, leds);

void setup()
{
    delay(3000); // power-up safety delay
    // It's important to set the color correction for your LED strip here,
    // so that colors can be more accurately rendered through the 'temperature' profiles
    FastLED_addLeds(STRIP_TYPE, LED_PIN, &leds, NUM_LEDS);
    FastLED_setCorrection(TypicalSMD5050);
    FastLED_setBrightness(BRIGHTNESS);
}

// FastLED provides two color-management controls:
//   (1) color correction settings for each LED strip, and
//   (2) master control of the overall output 'color temperature'
//
// FastLED provides these pre-configured incandescent color profiles:
//     Candle, Tungsten40W, Tungsten100W, Halogen, CarbonArc,
//     HighNoonSun, DirectSunlight, OvercastSky, ClearBlueSky,
// FastLED provides these pre-configured gaseous-light color profiles:
//     WarmFluorescent, StandardFluorescent, CoolWhiteFluorescent,
//     FullSpectrumFluorescent, GrowLightFluorescent, BlackLightFluorescent,
//     MercuryVapor, SodiumVapor, MetalHalide, HighPressureSodium,
// FastLED also provides an "Uncorrected temperature" profile
//    UncorrectedTemperature;
//
// THIS EXAMPLE demonstrates the second, "color temperature" control.
// It shows a simple rainbow animation first with one temperature profile,
// and a few seconds later, with a different temperature profile.
//
// The first pixel of the strip will show the color temperature.
//
// This version is slightly fancier than the original from FastLED.  It 
// loops through all of the color temperatures and refreshes the rainbow.
//

// How many seconds to show each temperature before switching
#define DISPLAYTIME 10
// How many seconds to show black between switches
#define BLACKTIME 2

#define NUMBER_TEMPERATURES 20

static unsigned color_temperatures[NUMBER_TEMPERATURES] = {
    Candle,
    Tungsten40W,
    Tungsten100W,
    Halogen,
    CarbonArc,
    HighNoonSun,
    DirectSunlight,
    OvercastSky,
    ClearBlueSky,
    WarmFluorescent,
    StandardFluorescent,
    CoolWhiteFluorescent,
    FullSpectrumFluorescent,
    GrowLightFluorescent,
    BlackLightFluorescent,
    MercuryVapor,
    SodiumVapor,
    MetalHalide,
    HighPressureSodium,
    UncorrectedTemperature};

void loop()
{
    static uint8_t starthue = 0;

    leds.fill_rainbow(&leds, --starthue, 2);

    // Choose which 'color temperature' profile to enable.
    uint8_t secs = (millis() / 1000) % (DISPLAYTIME * NUMBER_TEMPERATURES);
    FastLED_setTemperature(color_temperatures[secs / DISPLAYTIME]); // first temperature
    *(leds.get(&leds, 0)) = color_temperatures[secs / DISPLAYTIME];                     // show indicator pixel

    // Black out the LEDs for a few seconds between color changes
    // to let the eyes and brains adjust
    if ((secs % DISPLAYTIME) < BLACKTIME)
    {
        FastLED_clear(true);
    }

    FastLED_show();
    FastLED_delay(8);
}