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

#include <sys/neutrino.h>
#include "rpi_ws281x.h"
#include "color.h"
#include "crgb.h"
#include "lib8tion.h"
#include "colorpalettes.h"

// LED alias definitions (FastLED to rpi_ws281x, assuming typical color ordering)
// ## Clockless types ##
#define NEOPIXEL WS2811_STRIP_GRB
#define SM16703 WS2811_STRIP_RGB
#define TM1829 WS2811_STRIP_RGB
#define TM1812 WS2811_STRIP_RGB
#define TM1809 WS2811_STRIP_RGB
#define TM1804 WS2811_STRIP_RGB
#define TM1803 WS2811_STRIP_RGB
#define UCS1903 WS2811_STRIP_RGB
#define UCS1903B WS2811_STRIP_RGB
#define UCS1904 WS2811_STRIP_RGB
#define UCS2903 WS2811_STRIP_RGB
#define WS2812 WS2811_STRIP_GRB
#define WS2852 WS2811_STRIP_GRB
#define WS2812B WS2811_STRIP_GRB
#define GS1903 WS2811_STRIP_RGB
#define SK6812 SK6812_STRIP
#define SK6822 WS2811_STRIP_RGB
#define APA106 WS2811_STRIP_RGB
#define PL9823 WS2811_STRIP_RGB
#define SK6822 WS2811_STRIP_RGB
#define WS2811 WS2811_STRIP_RGB
#define WS2813 WS2811_STRIP_RGB
#define APA104 WS2811_STRIP_RGB
#define WS2811_400 WS2811_STRIP_RGB
#define GE8822 WS2811_STRIP_RGB
#define GW6205 WS2811_STRIP_RGB
#define GW6205_400 WS2811_STRIP_RGB
#define LPD1886 WS2811_STRIP_RGB
#define LPD1886_8BIT WS2811_STRIP_RGB

// ## Clocked (SPI) types ##
// (currently not supported)
#define LPD6803 WS2811_STRIP_GRB
#define LPD8806 WS2811_STRIP_GRB
#define WS2801 WS2811_STRIP_RGB
#define WS2803 WS2811_STRIP_RGB
#define SM16716 WS2811_STRIP_RGB
#define P9813 WS2811_STRIP_BGR
#define DOTSTAR WS2811_STRIP_BGR
#define APA102 WS2811_STRIP_BGR
#define SK9822 WS2811_STRIP_BGR

// structure and functions for RGBArray LED sets
typedef struct rgb_pixel_set
{
    int direction;
    int length;
    ws2811_led_t *leds;

    ws2811_led_t *(*get)(struct rgb_pixel_set *rps, unsigned index);
    void (*copyFrom)(struct rgb_pixel_set *rps_to, int to_start, int to_end,
                     struct rgb_pixel_set *rps_from, int from_start, int from_end);
    void (*nscale8)(struct rgb_pixel_set *rps, uint8_t scale);
    void (*fadeToBlackBy)(struct rgb_pixel_set *rps, uint8_t fadefactor);
    void (*fill_rainbow)(struct rgb_pixel_set *rps, uint8_t initialhue, uint8_t deltahue);
} rgb_pixel_set;

extern void nscale8(struct rgb_pixel_set *rps, uint8_t scale);
extern void fadeToBlackBy(struct rgb_pixel_set *rps, uint8_t fadefactor);
/// Fill all of the LEDs with a rainbow of colors.
/// @param initialhue the starting hue for the rainbow
/// @param deltahue how many hue values to advance for each LED
extern void fill_rainbow(struct rgb_pixel_set *rps, uint8_t initialhue, uint8_t deltahue);
extern ws2811_led_t *get(struct rgb_pixel_set *rps, unsigned index);
extern void copyFrom(struct rgb_pixel_set *rps_to, int to_start, int to_end,
                     struct rgb_pixel_set *rps_from, int from_start, int from_end);

extern ws2811_t ledset;

#define CRGBArray(size, name) rgb_pixel_set name =                      \
                                  {                                     \
                                      .direction = (size < 0) ? -1 : 1, \
                                      .length = size,                   \
                                      .leds = NULL,                     \
                                      .get = get,                       \
                                      .copyFrom = copyFrom,             \
                                      .nscale8 = nscale8,               \
                                      .fadeToBlackBy = fadeToBlackBy,   \
                                      .fill_rainbow = fill_rainbow}

/// Update all our controllers with the current led colors
#define FastLED_show() FastLED_showAt(FastLED_getBrightness())

/// Update all our controllers with the current led colors, using the passed in brightness
/// @param brightness the brightness value to use in place of the stored value
extern void FastLED_showAt(uint8_t brightness);

/// Delay for the given number of milliseconds.  Provided to allow the library to be used on platforms
/// that don't have a delay function (to allow code to be more portable).
/// @note This will call show() constantly to drive the dithering engine (and will call show() at least once).
/// @param ms the number of milliseconds to pause for
#define FastLED_delay(delay) nanospin_ns(delay * 1000)

/// Clear the leds, wiping the local array of data. Optionally you can also
/// send the cleared data to the LEDs.
/// @param write_data whether or not to write out to the leds as well
extern void FastLED_clear(bool write_data);

extern void FastLED_addLeds(unsigned strip_type, unsigned strip_pin, rgb_pixel_set *leds, unsigned number_leds);

/// Get the current global brightness setting
/// @returns the current global brightness value
extern uint8_t FastLED_getBrightness();

/// Set the global brightness scaling
/// @param scale a 0-255 value for how much to scale all leds before writing them out
extern void FastLED_setBrightness(uint8_t brightness);

/// Set the maximum power to be used, given in volts and milliamps.
/// @param volts how many volts the leds are being driven at (usually 5)
/// @param milliamps the maximum milliamps of power draw you want
extern void FastLED_setMaxPowerInVoltsAndMilliamps(uint8_t volts, uint32_t milliamps);

/// Set the maximum power to be used, given in milliwatts
/// @param milliwatts the max power draw desired, in milliwatts
extern void FastLED_setMaxPowerInMilliWatts(uint32_t milliwatts);

/// Set a global color correction.  Sets the color correction for all added led strips,
/// overriding whatever previous color correction those controllers may have had.
/// @param correction A CRGB value describing the color correction.
extern void FastLED_setCorrection(LEDColorCorrection color_correction);

/// Set a global color temperature.  Sets the color temperature for all added led strips,
/// overriding whatever previous color temperature those controllers may have had.
/// @param temp A CRGB value describing the color temperature
extern void FastLED_setTemperature(ws2811_led_t color_temperature);

/*
    /// Clear out the local data array
    void clearData();

    /// Set all leds on all controllers to the given color/scale.
    /// @param color what color to set the leds to
    /// @param scale what brightness scale to show at
    void showColor(const struct CRGB & color, uint8_t scale);

    /// Set all leds on all controllers to the given color
    /// @param color what color to set the leds to
    void showColor(const struct CRGB & color) { showColor(color, m_Scale); }

    void delay(unsigned long ms);

    /// Set the dithering mode.  Sets the dithering mode for all added led strips, overriding
    /// whatever previous dithering option those controllers may have had.
    /// @param ditherMode what type of dithering to use, either BINARY_DITHER or DISABLE_DITHER
    void setDither(uint8_t ditherMode = BINARY_DITHER);

    /// Set the maximum refresh rate.  This is global for all leds.  Attempts to
    /// call show() faster than this rate will simply wait.
    /// @note The refresh rate defaults to the slowest refresh rate of all the leds added through addLeds().
    /// If you wish to set/override this rate, be sure to call setMaxRefreshRate() _after_
    /// adding all of your leds.
    /// @param refresh maximum refresh rate in hz
    /// @param constrain constrain refresh rate to the slowest speed yet set
    void setMaxRefreshRate(uint16_t refresh, bool constrain=false);

    /// For debugging, this will keep track of time between calls to countFPS(). Every
    /// `nFrames` calls, it will update an internal counter for the current FPS.
    /// @todo Make this a rolling counter
    /// @param nFrames how many frames to time for determining FPS
    void countFPS(int nFrames=25);

    /// Get the number of frames/second being written out
    /// @returns the most recently computed FPS value
    uint16_t getFPS() { return m_nFPS; }

    /// Get how many controllers have been registered
    /// @returns the number of controllers (strips) that have been added with addLeds()
    int count();

    /// Get a reference to a registered controller
    /// @returns a reference to the Nth controller
    CLEDController & operator[](int x);

    /// Get the number of leds in the first controller
    /// @returns the number of LEDs in the first controller
    int size();

    /// Get a pointer to led data for the first controller
    /// @returns pointer to the CRGB buffer for the first controller
    CRGB *leds();


*/