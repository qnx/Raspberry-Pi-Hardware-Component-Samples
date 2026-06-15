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

#ifndef DC_MOTOR_H_INCLUDED
#define DC_MOTOR_H_INCLUDED

#include <stdbool.h>

typedef struct dc_motor_s {
    bool initialized;

    /**
     * PCA9685 PWM channel for DC motor -/+
     *
     * For motor 1 it's
     *  10, 9, 11, 12
     * For motor 2 it's
     *  4, 3, 5, 6
     * For motor 3 it's
     *  4, 3, 5, 6
     * For motor 4 it's
     *  4, 3, 5, 6
     */
    int positive;
    int negative;
} dc_motor_t;

#endif // DC_MOTOR_H_INCLUDED