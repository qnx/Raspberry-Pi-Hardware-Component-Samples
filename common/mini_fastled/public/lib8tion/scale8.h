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

#ifndef __INC_LIB8TION_SCALE8_H
#define __INC_LIB8TION_SCALE8_H

#include <sys/neutrino.h>
#include <stdint.h>
#include "lib8tion/types.h"
#include "crgb.h"

/// Fast, efficient 8-bit scaling functions specifically
/// designed for high-performance LED programming.
///
/// For RaspBerry PI 4, some of these functions are now defines
/// for the equivalent C expression.

/// Scale one byte by a second one, which is treated as
/// the numerator of a fraction whose denominator is 256.
///
/// In other words, it computes i * (scale / 256)
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @returns scaled value

#define scale8(i, s) ((i * s) >> 8)

/// The "video" version of scale8() guarantees that the output will
/// be only be zero if one or both of the inputs are zero.
/// If both inputs are non-zero, the output is guaranteed to be non-zero.
/// This makes for better "video"/LED dimming, at the cost of
/// several additional cycles.
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @returns scaled value
/// @see scale8()

#define scale8_video(i, scale) (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0)

/// These functions are more efficient for scaling multiple
/// bytes at once, but require calling cleanup_R1() afterwards.

/// This version of scale8() does not clean up the R1 register on AVR.
/// If you are doing several "scale8()'s" in a row, use this, and
/// then explicitly call cleanup_R1().
/// @warning You **MUST** call cleanup_R1() after using this function!
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @returns scaled value
/// @see scale8()

#define scale8_LEAVING_R1_DIRTY(i, s) ((i * s) >> 8)

/// In place modifying version of scale8() that does not clean up the R1
/// register on AVR. If you are doing several "scale8()'s" in a row, use this,
/// and then explicitly call cleanup_R1().
/// @warning You **MUST** call cleanup_R1() after using this function!
/// @par
/// @warning This function always modifies its arguments in place!
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @see scale8()

#define nscale8_LEAVING_R1_DIRTY(i, s) ((i * s) >> 8)

/// This version of scale8_video() does not clean up the R1 register on AVR.
/// If you are doing several "scale8_video()'s" in a row, use this, and
/// then explicitly call cleanup_R1().
/// @warning You **MUST** call cleanup_R1() after using this function!
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @returns scaled value
/// @see scale8_video()

#define scale8_video_LEAVING_R1_DIRTY(i, scale) (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0)

/// In place modifying version of scale8_video() that does not clean up the R1
/// register on AVR. If you are doing several "scale8_video()'s" in a row, use
/// this, and then explicitly call cleanup_R1().
/// @warning You **MUST** call cleanup_R1() after using this function!
/// @par
/// @warning This function always modifies its arguments in place!
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @see scale8_video()

#define nscale8_video_LEAVING_R1_DIRTY(i, scale) (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0)

/// Clean up the r1 register after a series of *LEAVING_R1_DIRTY calls
/// @ingroup ScalingDirty
#define cleanup_R1() 

/// Scale three one-byte values by a fourth one, which is treated as
/// the numerator of a fraction whose demominator is 256.
///
/// In other words, it computes r,g,b * (scale / 256)
///
/// @warning This function always modifies its arguments in place!
/// @param r first value to scale
/// @param g second value to scale
/// @param b third value to scale
/// @param scale scale factor, in n/256 units

extern void nscale8x3(uint8_t *red, uint8_t *green, uint8_t *blue, fract8 scale);

/// Scale three one-byte values by a fourth one, which is treated as
/// the numerator of a fraction whose demominator is 256.
///
/// In other words, it computes r,g,b * (scale / 256), ensuring
/// that non-zero values passed in remain non-zero, no matter how low the scale
/// argument.
///
/// @warning This function always modifies its arguments in place!
/// @param r first value to scale
/// @param g second value to scale
/// @param b third value to scale
/// @param scale scale factor, in n/256 units

extern void nscale8x3_video(uint8_t *red, uint8_t *green, uint8_t *blue, fract8 scale);

/// Scale two one-byte values by a third one, which is treated as
/// the numerator of a fraction whose demominator is 256.
///
/// In other words, it computes i,j * (scale / 256).
///
/// @warning This function always modifies its arguments in place!
/// @param i first value to scale
/// @param j second value to scale
/// @param scale scale factor, in n/256 units

extern void nscale8x2(uint8_t *i, uint8_t *j, fract8 scale);

/// Scale two one-byte values by a third one, which is treated as
/// the numerator of a fraction whose demominator is 256.
///
/// In other words, it computes i,j * (scale / 256), ensuring
/// that non-zero values passed in remain non zero, no matter how low the scale
/// argument.
///
/// @warning This function always modifies its arguments in place!
/// @param i first value to scale
/// @param j second value to scale
/// @param scale scale factor, in n/256 units

extern void nscale8x2_video(uint8_t *i, uint8_t *j, fract8 scale);

/// Scale a 16-bit unsigned value by an 8-bit value, which is treated
/// as the numerator of a fraction whose denominator is 256.
///
/// In other words, it computes i * (scale / 256)
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @returns scaled value

#define scale16by8(i, s) ((i * s) >> 8)

/// Scale a 16-bit unsigned value by an 16-bit value, which is treated
/// as the numerator of a fraction whose denominator is 65536.
/// In other words, it computes i * (scale / 65536)
/// @param i input value to scale
/// @param scale scale factor, in n/65536 units
/// @returns scaled value

#define scale16(i, s) ((i * s) >> 16)

/// The eye does not respond in a linear way to light.
/// High speed PWM'd LEDs at 50% duty cycle appear far
/// brighter then the "half as bright" you might expect.
///
/// If you want your midpoint brightness LEDs (128) to
/// appear half as bright as "full" brightness (255), you
/// have to apply a "dimming function".
///
/// @note These are approximations of gamma correction with
///       a gamma value of 2.0.
/// @see @ref GammaFuncs

/// Adjust a scaling value for dimming.
/// @see scale8()

#define dim8_raw(x) scale8(x, x)

/// Adjust a scaling value for dimming for video (value will never go below 1)
/// @see scale8_video()

#define dim8_video(x) scale8_video(x, x)

/// Linear version of the dimming function that halves for values < 128
extern uint8_t dim8_lin(uint8_t x);

/// Brighten a value (inverse of dim8_raw())
extern uint8_t brighten8_raw(uint8_t x);

/// Brighten a value (inverse of dim8_video())
extern uint8_t brighten8_video(uint8_t x);

/// Brighten a value (inverse of dim8_lin())
extern uint8_t brighten8_lin(uint8_t x);

#endif