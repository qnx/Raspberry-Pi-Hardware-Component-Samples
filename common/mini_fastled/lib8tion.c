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
#include "lib8tion.h"

// math operations
uint8_t qadd8( uint8_t i, uint8_t j)
{
    unsigned int t = i + j;
    if( t > 255) t = 255;
    return t;
}

 int8_t qadd7( int8_t i, int8_t j)
{
    int16_t t = i + j;
    if( t > 127) t = 127;
    return t;
}

 uint8_t qsub8( uint8_t i, uint8_t j)
{
    int t = i - j;
    if( t < 0) t = 0;
    return t;
}

 uint8_t add8( uint8_t i, uint8_t j)
{
    int t = i + j;
    return t;
}

 uint16_t add8to16( uint8_t i, uint16_t j)
{
    uint16_t t = i + j;
    return t;
}

 uint8_t sub8( uint8_t i, uint8_t j)
{
    int t = i - j;
    return t;
}

 uint8_t avg8( uint8_t i, uint8_t j)
{
    return (i + j) >> 1;
}

 uint16_t avg16( uint16_t i, uint16_t j)
{
    return (uint32_t)((uint32_t)(i) + (uint32_t)(j)) >> 1;
}

 int8_t avg7( int8_t i, int8_t j)
{
    return ((i + j) >> 1) + (i & 0x1);
}

 int16_t avg15( int16_t i, int16_t j)
{
    return ((int32_t)((int32_t)(i) + (int32_t)(j)) >> 1) + (i & 0x1);
}

 uint8_t mod8( uint8_t a, uint8_t m)
{
    while( a >= m) a -= m;
    return a;
}

 uint8_t addmod8( uint8_t a, uint8_t b, uint8_t m)
{
    a += b;
    while( a >= m) a -= m;
    return a;
}

 uint8_t submod8( uint8_t a, uint8_t b, uint8_t m)
{
    a -= b;
    while( a >= m) a -= m;
    return a;
}

 uint8_t mul8( uint8_t i, uint8_t j)
{
    return ((int)i * (int)(j) ) & 0xFF;
}

 uint8_t qmul8( uint8_t i, uint8_t j)
{
    int p = ((int)i * (int)(j) );
    if( p > 255) p = 255;
    return p;
}

 int8_t abs8( int8_t i)
{
    if( i < 0) i = -i;
    return i;
}

uint8_t sqrt16(uint16_t x)
{
    if( x <= 1) {
        return x;
    }

    uint8_t low = 1; // lower bound
    uint8_t hi, mid;

    if( x > 7904) {
        hi = 255;
    } else {
        hi = (x >> 5) + 8; // initial estimate for upper bound
    }

    do {
        mid = (low + hi) >> 1;
        if ((uint16_t)(mid * mid) > x) {
            hi = mid - 1;
        } else {
            if( mid == 255) {
                return 255;
            }
            low = mid + 1;
        }
    } while (hi >= low);

    return low - 1;
}

uint8_t blend8( uint8_t a, uint8_t b, uint8_t amountOfB)
{
    uint16_t partial;
    uint8_t result;

    uint8_t amountOfA = 255 - amountOfB;

    partial = (a * amountOfA);
    partial += (b * amountOfB);
    result = partial >> 8;

    return result;

}

/// scaling functions
void nscale8x3(uint8_t *red, uint8_t *green, uint8_t *blue, fract8 scale)
{
    *red = ((int)*red * (int)scale) >> 8;
    *green = ((int)*green * (int)scale) >> 8;
    *blue = ((int)*blue * (int)scale) >> 8;
}

void nscale8x3_video(uint8_t *red, uint8_t *green, uint8_t *blue, fract8 scale)
{
    *red = (((int)*red * (int)scale) >> 8) + (((int)*red && scale) ? 1 : 0);
    *green = (((int)*green * (int)scale) >> 8) + (((int)*green && scale) ? 1 : 0);
    *blue = (((int)*blue * (int)scale) >> 8) + (((int)*blue && scale) ? 1 : 0);
}

void nscale8x2(uint8_t *i, uint8_t *j, fract8 scale)
{
    *i = ((int)*i * (int)scale) >> 8;
    *j = ((int)*j * (int)scale) >> 8;
}

void nscale8x2_video(uint8_t *i, uint8_t *j, fract8 scale)
{
    *i = (((int)*i * (int)scale) >> 8) + (((int)*i && scale) ? 1 : 0);
    *j = (((int)*j * (int)scale) >> 8) + (((int)*j && scale) ? 1 : 0);
}

// dimming functions
uint8_t dim8_lin(uint8_t x)
{
    if (x & 0x80)
    {
        x = scale8(x, x);
    }
    else
    {
        x += 1;
        x /= 2;
    }
    return x;
}

uint8_t brighten8_raw(uint8_t x)
{
    uint8_t ix = 255 - x;
    return 255 - scale8(ix, ix);
}

uint8_t brighten8_video(uint8_t x)
{
    uint8_t ix = 255 - x;
    return 255 - scale8_video(ix, ix);
}

uint8_t brighten8_lin(uint8_t x)
{
    uint8_t ix = 255 - x;
    if (ix & 0x80)
    {
        ix = scale8(ix, ix);
    }
    else
    {
        ix += 1;
        ix /= 2;
    }
    return 255 - ix;
}

// mapping functions
uint8_t map8(uint8_t in, uint8_t rangeStart, uint8_t rangeEnd)
{
    uint8_t rangeWidth = rangeEnd - rangeStart;
    uint8_t out = scale8(in, rangeWidth);
    out += rangeStart;
    return out;
}

// random functions
/// Seed for the random number generator functions

// random functions
/// Seed for the random number generator functions
uint16_t rand16seed; // = RAND16_SEED;

uint8_t random8()
{
    rand16seed = APPLY_FASTLED_RAND16_2053(rand16seed) + FASTLED_RAND16_13849;
    // return the sum of the high and low bytes, for better
    //  mixing and non-sequential correlation
    return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) +
                     ((uint8_t)(rand16seed >> 8)));
}

uint8_t random8_lim(uint8_t lim)
{
    uint8_t r = random8();
    r = (r * lim) >> 8;
    return r;
}

uint8_t random8_range(uint8_t min, uint8_t lim)
{
    uint8_t delta = lim - min;
    uint8_t r = random8_lim(delta) + min;
    return r;
}

uint16_t random16()
{
    rand16seed = APPLY_FASTLED_RAND16_2053(rand16seed) + FASTLED_RAND16_13849;
    return rand16seed;
}

uint16_t random16_lim(uint16_t lim)
{
    uint16_t r = random16();
    uint32_t p = (uint32_t)lim * (uint32_t)r;
    r = p >> 16;
    return r;
}

uint16_t random16_range(uint16_t min, uint16_t lim)
{
    uint16_t delta = lim - min;
    uint16_t r = random16_lim(delta) + min;
    return r;
}

// conversion functions
float sfract15ToFloat(sfract15 y)
{
    return y / 32768.0;
}

sfract15 floatToSfract15(float f)
{
    return f * 32768.0;
}

// waveform function
uint16_t beat88(accum88 beats_per_minute_88, uint32_t timebase)
{
    // BPM is 'beats per minute', or 'beats per 60000ms'.
    // To avoid using the (slower) division operator, we
    // want to convert 'beats per 60000ms' to 'beats per 65536ms',
    // and then use a simple, fast bit-shift to divide by 65536.
    //
    // The ratio 65536:60000 is 279.620266667:256; we'll call it 280:256.
    // The conversion is accurate to about 0.05%, more or less,
    // e.g. if you ask for "120 BPM", you'll get about "119.93".
    return (((GET_MILLIS())-timebase) * beats_per_minute_88 * 280) >> 16;
}

uint16_t beat16(accum88 beats_per_minute, uint32_t timebase)
{
    // Convert simple 8-bit BPM's to full Q8.8 accum88's if needed
    if (beats_per_minute < 256)
        beats_per_minute <<= 8;
    return beat88(beats_per_minute, timebase);
}

uint8_t beat8(accum88 beats_per_minute, uint32_t timebase)
{
    return beat16(beats_per_minute, timebase) >> 8;
}

uint16_t beatsin88_ext(accum88 beats_per_minute_88, uint16_t lowest, uint16_t highest,
                   uint32_t timebase, uint16_t phase_offset)
{
    uint16_t beat = beat88(beats_per_minute_88, timebase);
    uint16_t beatsin = (sin16(beat + phase_offset) + 32768);
    uint16_t rangewidth = highest - lowest;
    uint16_t scaledbeat = scale16(beatsin, rangewidth);
    uint16_t result = lowest + scaledbeat;
    return result;
}

uint16_t beatsin16_ext(accum88 beats_per_minute, uint16_t lowest, uint16_t highest,
                       uint32_t timebase, uint16_t phase_offset)
{
    uint16_t beat = beat16(beats_per_minute, timebase);
    uint16_t beatsin = (sin16(beat + phase_offset) + 32768);
    uint16_t rangewidth = highest - lowest;
    uint16_t scaledbeat = scale16(beatsin, rangewidth);
    uint16_t result = lowest + scaledbeat;
    return result;
}

uint8_t beatsin8_ext(accum88 beats_per_minute, uint8_t lowest, uint8_t highest,
                     uint32_t timebase, uint8_t phase_offset)
{
    uint8_t beat = beat8(beats_per_minute, timebase);
    uint8_t beatsin = sin8(beat + phase_offset);
    uint8_t rangewidth = highest - lowest;
    uint8_t scaledbeat = scale8(beatsin, rangewidth);
    uint8_t result = lowest + scaledbeat;
    return result;
}

// timekeeping functions
uint64_t run_millis = 0;
uint64_t last_run_millis = 0;
uint64_t last_run_secs = 0;

uint64_t micros()
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    uint64_t ms = tp.tv_sec * 1000000 + tp.tv_nsec / 1000; // get microseconds
    return ms;
}

uint64_t millis()
{
    return micros() / 1000;
}
