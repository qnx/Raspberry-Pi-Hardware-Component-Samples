/**
* Copyright (c) 2026, BlackBerry Limited. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef STEPPER_MOTOR_H_INCLUDED
#define STEPPER_MOTOR_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#define MAX_MICROSTEPS 32

typedef struct stepper_motor_s {
    bool initialized;
    int microsteps; // default 16
    int current_microstep;

    // Assumes max microsteps of 32
    uint16_t curve[MAX_MICROSTEPS];

    /**
     * PCA9685 PWM channel for stepper motor a-/+ b-/+ coils
     *
     * For stepper 1 it's
     *  10, 9, 11, 12
     * For stepper 2 it's
     *  4, 3, 5, 6
     */
    int a1;
    int a2;
    int b1;
    int b2;
} stepper_motor_t;

/**
 * Which direction the stepper will rotate
 * 
 * Note the physical direction CW or CCW depends on the wiring of the stepper motor
 * and if backwards switching the +ve and -ve leads should fix it.
 */
typedef enum stepper_direction_e {
    STEPPER_DIR_FORWARD,
    STEPPER_DIR_BACKWARDS
} stepper_direction_t;

/**
 * How the stepper should move.
 */
typedef enum stepper_style_e {
    STEPPER_STYLE_SINGLE,
    STEPPER_STYLE_DOUBLE,
    STEPPER_STYLE_INTERLEAVE,
    STEPPER_STYLE_MICROSTEP,
} stepper_style_t;

#endif // STEPPER_MOTOR_H_INCLUDED
