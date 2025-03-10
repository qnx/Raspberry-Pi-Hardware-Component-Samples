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

#ifndef __INC_LIBI8TION_RANDOM_H
#define __INC_LIBI8TION_RANDOM_H

#include <sys/neutrino.h>

/// Fast, efficient random number generators specifically
/// designed for high-performance LED programming.

/// Fast 8-bit and 16-bit unsigned random number generators.
/// Significantly faster than Arduino random(), but
/// also somewhat less random.  You can add entropy.
///
/// Pseudo-random number generation follows the form:
///   @code
///   X(n+1) = (2053 * X(n)) + 13849)
///   @endcode

/// Multiplier value for pseudo-random number generation
#define FASTLED_RAND16_2053 ((uint16_t)(2053))
/// Increment value for pseudo-random number generation
#define FASTLED_RAND16_13849 ((uint16_t)(13849))

/// Multiplies a value by the pseudo-random multiplier
#define APPLY_FASTLED_RAND16_2053(x) (x * FASTLED_RAND16_2053)

/// Seed for the random number generator functions
extern uint16_t rand16seed; // = RAND16_SEED;

/// Set the 16-bit seed used for the random number generator
#define random16_set_seed(seed) rand16seed = seed;

/// Get the current seed value for the random number generator
#define random16_get_seed rand16seed

/// Add entropy into the random number generator
#define random16_add_entropy(entropy) rand16seed += entropy

/// Generate an 8-bit random number
/// @returns random 8-bit number, in the range 0-255
extern uint8_t random8();

/// Generate an 8-bit random number between 0 and lim
/// @param lim the upper bound for the result, exclusive
extern uint8_t random8_lim(uint8_t lim);

/// Generate an 8-bit random number in the given range
/// @param min the lower bound for the random number, inclusive
/// @param lim the upper bound for the random number, exclusive
extern uint8_t random8_range(uint8_t min, uint8_t lim);

/// Generate a 16-bit random number
/// @returns random 16-bit number, in the range 0-65535
extern uint16_t random16();

/// Generate an 16-bit random number between 0 and lim
/// @param lim the upper bound for the result, exclusive
extern uint16_t random16_lim(uint16_t lim);

/// Generate an 16-bit random number in the given range
/// @param min the lower bound for the random number, inclusive
/// @param lim the upper bound for the random number, exclusive
extern uint16_t random16_range(uint16_t min, uint16_t lim);

#endif