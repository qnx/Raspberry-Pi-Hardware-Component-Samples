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
#include "power_mgt.h"

/// Functions to limit the power used by FastLED

// POWER MANAGEMENT

/// These power usage values are approximate, and your exact readings
/// will be slightly (10%?) different from these.
///
/// They were arrived at by actually measuring the power draw of a number
/// of different LED strips, and a bunch of closed-loop-feedback testing
/// to make sure that if we USE these values, we stay at or under
/// the target power consumption.
/// Actual power consumption is much, much more complicated and has
/// to include things like voltage drop, etc., etc.
/// However, this is good enough for most cases, and almost certainly better
/// than no power management at all.
///
/// You're welcome to adjust these values as needed; there may eventually be an API
/// for changing these on the fly, but it saves codespace and RAM to have them
/// be compile-time constants.
/// @{
static const uint8_t gRed_mW = 16 * 5;   ///< 16mA @ 5v = 80mW
static const uint8_t gGreen_mW = 11 * 5; ///< 11mA @ 5v = 55mW
static const uint8_t gBlue_mW = 15 * 5;  ///< 15mA @ 5v = 75mW
static const uint8_t gDark_mW = 1 * 5;   ///<  1mA @ 5v =  5mW
/// @}

// Alternate calibration by RAtkins via pre-PSU wattage measurments;
// these are all probably about 20%-25% too high due to PSU heat losses,
// but if you're measuring wattage on the PSU input side, this may
// be a better set of calibrations.  (WS2812B)
//  static const uint8_t gRed_mW   = 100;
//  static const uint8_t gGreen_mW =  48;
//  static const uint8_t gBlue_mW  = 100;
//  static const uint8_t gDark_mW  =  12;

/// Debug Option: Set to 1 to enable the power limiting LED
/// @see set_max_power_indicator_LED()
#define POWER_LED 1

/// Debug Option: Set to enable Serial debug statements for power limit functions
#define POWER_DEBUG_PRINT 0

// Power consumed by the MCU
static const uint8_t gMCU_mW = 25 * 5; // 25mA @ 5v = 125 mW

static uint8_t gMaxPowerIndicatorLEDPinNumber = LED_CHANNEL_0_DATA_PIN; // default

uint32_t calculate_unscaled_power_mW(const CRGB *ledbuffer, uint16_t numLeds) // 25354
{
    uint32_t red32 = 0, green32 = 0, blue32 = 0;
    
    for (int index = 0; index < numLeds; index++)
    {
        red32 += CRGBW_red(ledbuffer[index]);
        green32 += CRGBW_green(ledbuffer[index]);
        blue32 += CRGBW_blue(ledbuffer[index]);
    }

    red32 *= gRed_mW;
    green32 *= gGreen_mW;
    blue32 *= gBlue_mW;

    red32 >>= 8;
    green32 >>= 8;
    blue32 >>= 8;

    uint32_t total = red32 + green32 + blue32 + (gDark_mW * numLeds);

    return total;
}

uint8_t calculate_max_brightness_for_power_vmA(const CRGB* ledbuffer, uint16_t numLeds, uint8_t target_brightness, uint32_t max_power_V, uint32_t max_power_mA) {
    return calculate_max_brightness_for_power_mW(ledbuffer, numLeds, target_brightness, max_power_V * max_power_mA);
}

uint8_t calculate_max_brightness_for_power_mW(const CRGB* ledbuffer, uint16_t numLeds, uint8_t target_brightness, uint32_t max_power_mW) {
    uint32_t total_mW = calculate_unscaled_power_mW( ledbuffer, numLeds);

   uint32_t requested_power_mW = ((uint32_t)total_mW * target_brightness) / 256;

   uint8_t recommended_brightness = target_brightness;
   if(requested_power_mW > max_power_mW) { 
       recommended_brightness = (uint32_t)((uint8_t)(target_brightness) * (uint32_t)(max_power_mW)) / ((uint32_t)(requested_power_mW));
   }

   return recommended_brightness;
}

uint8_t calculate_max_brightness_for_power_mW_ledset(ws2811_t *ledset, uint8_t target_brightness, uint32_t max_power_mW)
{
    uint32_t total_mW = 0;

    if (ledset->channel[0].gpionum != -1)
    {
        if (ledset->channel[0].leds)
        {
            total_mW += calculate_unscaled_power_mW(ledset->channel[0].leds, ledset->channel[0].count);
        }
    }

    if (ledset->channel[1].gpionum != -1)
    {
        if (ledset->channel[1].leds)
        {
            total_mW += calculate_unscaled_power_mW(ledset->channel[1].leds, ledset->channel[1].count);
        }
    }

    uint32_t requested_power_mW = ((uint32_t)total_mW * target_brightness) / 256;

    uint8_t recommended_brightness = target_brightness;
    if (requested_power_mW > max_power_mW)
    {
        recommended_brightness = (uint32_t)((uint8_t)(target_brightness) * (uint32_t)(max_power_mW)) / ((uint32_t)(requested_power_mW));
    }

    return recommended_brightness;
}

/*
// sets brightness to
//  - no more than target_brightness
//  - no more than max_mW milliwatts
uint8_t calculate_max_brightness_for_power_mW(uint8_t target_brightness, uint32_t max_power_mW)
{
    uint32_t total_mW = gMCU_mW;

    CLEDController *pCur = CLEDController::head();
    while (pCur)
    {
        total_mW += calculate_unscaled_power_mW(pCur->leds(), pCur->size());
        pCur = pCur->next();
    }

#if POWER_DEBUG_PRINT == 1
    Serial.print("power demand at full brightness mW = ");
    Serial.println(total_mW);
#endif

    uint32_t requested_power_mW = ((uint32_t)total_mW * target_brightness) / 256;
#if POWER_DEBUG_PRINT == 1
    if (target_brightness != 255)
    {
        Serial.print("power demand at scaled brightness mW = ");
        Serial.println(requested_power_mW);
    }
    Serial.print("power limit mW = ");
    Serial.println(max_power_mW);
#endif

    if (requested_power_mW < max_power_mW)
    {
#if POWER_LED > 0
        if (gMaxPowerIndicatorLEDPinNumber)
        {
            Pin(gMaxPowerIndicatorLEDPinNumber).lo(); // turn the LED off
        }
#endif
#if POWER_DEBUG_PRINT == 1
        Serial.print("demand is under the limit");
#endif
        return target_brightness;
    }

    uint8_t recommended_brightness = (uint32_t)((uint8_t)(target_brightness) * (uint32_t)(max_power_mW)) / ((uint32_t)(requested_power_mW));
#if POWER_DEBUG_PRINT == 1
    Serial.print("recommended brightness # = ");
    Serial.println(recommended_brightness);

    uint32_t resultant_power_mW = (total_mW * recommended_brightness) / 256;
    Serial.print("resultant power demand mW = ");
    Serial.println(resultant_power_mW);

    Serial.println();
#endif

#if POWER_LED > 0
    if (gMaxPowerIndicatorLEDPinNumber)
    {
        Pin(gMaxPowerIndicatorLEDPinNumber).hi(); // turn the LED on
    }
#endif

    return recommended_brightness;
}

void set_max_power_indicator_LED( uint8_t pinNumber)
{
    gMaxPowerIndicatorLEDPinNumber = pinNumber;
}

void set_max_power_in_volts_and_milliamps( uint8_t volts, uint32_t milliamps)
{
    FastLED.setMaxPowerInVoltsAndMilliamps(volts, milliamps);
}

void set_max_power_in_milliwatts( uint32_t powerInmW)
{
    FastLED.setMaxPowerInMilliWatts(powerInmW);
}

void show_at_max_brightness_for_power()
{
    // power management usage is now in FastLED.show, no need for this function
    FastLED.show();
}

void delay_at_max_brightness_for_power( uint16_t ms)
{
    FastLED.delay(ms);
}
*/