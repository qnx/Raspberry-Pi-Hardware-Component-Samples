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

/** Based on adafruit circuit python
 * (https://github.com/adafruit/Adafruit_CircuitPython_Motor/blob/main/adafruit_motor/)
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Scott Shawcroft for Adafruit Industries LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "motor_kit/motor_kit.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Updates the coils for the current step
 *
 * @note This implementation assumes PWM pins are used
 *
 * @param motor_kit Owner of stepper
 * @param stepper  Stepper for coils to be updated
 * @param microstepping If the motor is microstepping
 *
 * @return EOK on success, otherwise an error code.
 */
static int update_coils(motor_kit_t *motor_kit, stepper_motor_t *stepper,
                        bool microstepping);

/**
 * @brief Sets the duty cycle for a given pwm channel.
 *
 * @note Even through the PCA9685 is only 12-bit the full 16-bit duty_cycle will
 *       be used and shifted down before sending.
 *
 * @param motor_kit Owner of the PWM channel
 * @param duty_cycle 0-0xFFFF where 0 is completely off and 0xFFFF completely
 * one.
 *
 * @return EOK on success, otherwise an error code.
 */
static int set_pwm_duty_cycle(motor_kit_t *motor_kit, int pwm_channel,
                              uint16_t duty_cycle);

/**
 * Adafruits implementation relies on mod always returning a positive value
 * which is done by default in python In C/C++ it is possible to get negative
 * values for mode which means we must ensure it is always the positive value
 */
static int positive_modulo(int d, int mod) { return ((d % mod) + mod) % mod; }

int motor_kit_init(motor_kit_t *motor_kit, int i2c_bus, int i2c_addr,
                   int freq) {
  if (motor_kit == NULL) {
    return EINVAL;
  }
  memset(motor_kit, 0, sizeof(*motor_kit));

  int res = pwm_driver_pca9685_init(&motor_kit->driver, i2c_bus, i2c_addr);
  if (res != EOK) {
    printf("Failed to init pwm driver");
    return res;
  }

  res = pwm_driver_pca9685_set_frequency(&motor_kit->driver, freq);
  if (res != EOK) {
    printf("Failed to set PWM frequency");
    return res;
  }

  return EOK;
}

/**
 * @brief Destroy motor kit and any configured devices
 *
 * @note The object should be considered invalid after this call. Any usage is
 * considered UB.
 * @param motor_kit Motor kit to destroy
 *
 */
void motor_kit_destroy(motor_kit_t *motor_kit) {
  if (motor_kit == NULL) {
    return;
  }
  pwm_driver_pca9685_destroy(&motor_kit->driver);
  memset(motor_kit, 0, sizeof(*motor_kit));
}

int motor_kit_stepper_init(motor_kit_t *motor_kit, stepper_motor_t *stepper) {
  if (motor_kit == NULL || stepper == NULL) {
    return EINVAL;
  }

  // Before starting using the stepper the adafruit library will set the duty
  // cycle to fully on
  int channel1, channel2;

  if (&motor_kit->stepper1 == stepper) {
    if (motor_kit->motor1.initialized || motor_kit->motor2.initialized) {
      return EBUSY;
    }
    channel1 = 8;
    channel2 = 13;
    stepper->a1 = 10;
    stepper->a2 = 9;
    stepper->b1 = 11;
    stepper->b2 = 12;
  } else if (&motor_kit->stepper2 == stepper) {
    if (motor_kit->motor3.initialized || motor_kit->motor4.initialized) {
      return EBUSY;
    }
    channel1 = 7;
    channel2 = 2;
    stepper->a1 = 4;
    stepper->a2 = 3;
    stepper->b1 = 5;
    stepper->b2 = 6;
  } else {
    return ENODEV; // This stepper doesn't belong to this motor kit....
  }

  int res = EOK;
  res |= set_pwm_duty_cycle(motor_kit, channel1, 0xFFFF);
  res |= set_pwm_duty_cycle(motor_kit, channel2, 0xFFFF);

  if (res == EOK) {
    stepper->initialized = true;
    stepper->current_microstep = 0;
    stepper->microsteps = 8;

    // Calculate the step curve
    for (int i = 0; i < stepper->microsteps + 1; i++) {
      stepper->curve[i] = 0xFFFF * sin(M_PI / (2 * stepper->microsteps) * i);
    }
    res = update_coils(motor_kit, stepper, false);
  }
  return res;
}

int motor_kit_stepper_step(motor_kit_t *motor_kit, stepper_motor_t *stepper,
                           stepper_direction_t dir, stepper_style_t style) {
  int step_size = 0;
  if (style == STEPPER_STYLE_MICROSTEP) {
    step_size = 1;
  } else {
    /**
     * Because we support both micro stepping and full steps it is possible
     * someone is switching between the 2 in which case we may need to
     * compensate to align back with the patten
     */
    int half_step = stepper->microsteps / 2;
    int align_steps = positive_modulo(stepper->current_microstep, half_step);
    if (align_steps != 0) {
      step_size = 0;
      if (dir == STEPPER_DIR_FORWARD) {
        // Move it to the next step
        stepper->current_microstep += half_step - align_steps;
      } else {
        // Move it back to the current step
        stepper->current_microstep -= align_steps;
      }
    } else if (style == STEPPER_STYLE_INTERLEAVE) {
      step_size = half_step;
    }

    int current_interleave = stepper->current_microstep / half_step;
    if ((style == STEPPER_STYLE_SINGLE &&
         (positive_modulo(current_interleave, 2) == 1)) ||
        (style == STEPPER_STYLE_DOUBLE &&
         (positive_modulo(current_interleave, 2) == 0))) {
      step_size = half_step;
    } else if (style == STEPPER_STYLE_DOUBLE || style == STEPPER_STYLE_SINGLE) {
      step_size = stepper->microsteps;
    }
  }

  if (dir == STEPPER_DIR_FORWARD) {
    stepper->current_microstep += step_size;
  } else {
    stepper->current_microstep -= step_size;
  }

  update_coils(motor_kit, stepper, style == STEPPER_STYLE_MICROSTEP);
  return EOK;
}

int motor_kit_stepper_release(motor_kit_t *motor_kit,
                              stepper_motor_t *stepper) {
  if (motor_kit == NULL || stepper == NULL || !stepper->initialized) {
    return EINVAL;
  }
  int res = EOK;
  res |= set_pwm_duty_cycle(motor_kit, stepper->a2, 0);
  res |= set_pwm_duty_cycle(motor_kit, stepper->b1, 0);
  res |= set_pwm_duty_cycle(motor_kit, stepper->a1, 0);
  res |= set_pwm_duty_cycle(motor_kit, stepper->b2, 0);
  return res;
}

int motor_kit_dc_init(motor_kit_t *motor_kit, dc_motor_t *motor) {
  if (motor_kit == NULL || motor == NULL || motor->initialized) {
    return EINVAL;
  }

  int channel = 0; // Channel to be energized to turn motor on
  if (&motor_kit->motor1 == motor) {
    if (motor_kit->stepper1.initialized) {
      return EBUSY;
    }
    channel = 8;
    motor->negative = 9;
    motor->positive = 10;
  } else if (&motor_kit->motor2 == motor) {
    if (motor_kit->stepper1.initialized) {
      return EBUSY;
    }
    channel = 13;
    motor->negative = 11;
    motor->positive = 12;
  } else if (&motor_kit->motor3 == motor) {
    if (motor_kit->stepper2.initialized) {
      return EBUSY;
    }
    channel = 2;
    motor->negative = 3;
    motor->positive = 4;
  } else if (&motor_kit->motor4 == motor) {
    if (motor_kit->stepper2.initialized) {
      return EBUSY;
    }
    channel = 7;
    motor->negative = 5;
    motor->positive = 6;
  } else {
    return ENODEV; // This motor doesn't belong to this motor kit....
  }

  int res = set_pwm_duty_cycle(motor_kit, channel, 0xFFFF);
  if (res == EOK) {
    motor->initialized = true;
  }
  return res;
}

int motor_kit_dc_throttle(motor_kit_t *motor_kit, dc_motor_t *motor,
                          int8_t throttle) {
  if (motor_kit == NULL || motor == NULL || !motor->initialized) {
    return EINVAL;
  }

  // Validate range
  if (throttle < -100 || throttle > 100) {
    return EINVAL;
  }

  int res = EOK;
  int duty_cycle = 0xFFFF * (throttle / 100.0);
  if (throttle == 0) {
    // Brake mode
    res |= set_pwm_duty_cycle(motor_kit, motor->negative, 0xFFFF);
    res |= set_pwm_duty_cycle(motor_kit, motor->positive, 0xFFFF);
  } else if (throttle < 0) { // Reverse direction
    res |= set_pwm_duty_cycle(motor_kit, motor->negative, duty_cycle);
    res |= set_pwm_duty_cycle(motor_kit, motor->positive, 0);
  } else { // Forward direction
    res |= set_pwm_duty_cycle(motor_kit, motor->negative, 0);
    res |= set_pwm_duty_cycle(motor_kit, motor->positive, duty_cycle);
  }
  return res;
}

int motor_kit_dc_release(motor_kit_t *motor_kit, dc_motor_t *motor) {
  if (motor_kit == NULL || motor == NULL || !motor->initialized) {
    return EINVAL;
  }

  int res = EOK;
  res |= set_pwm_duty_cycle(motor_kit, motor->negative, 0);
  res |= set_pwm_duty_cycle(motor_kit, motor->positive, 0);
  return res;
}

static int set_pwm_duty_cycle(motor_kit_t *motor_kit, int pwm_channel,
                              uint16_t duty_cycle) {

  int low = duty_cycle;
  int high = 0;

  /**
   * Special cases for fully on and fully off.
   *
   * Logic based on:
   * https://github.com/adafruit/Adafruit_CircuitPython_PCA9685/blob/main/adafruit_pca9685.py#L87
   */
  if (duty_cycle < 0x0010) { // Fully off
    low = 0x1000;
    high = 0;
  } else if (duty_cycle == 0xFFFF) { // fully on
    low = 0;
    high = 0x1000;
  } else {
    // PCA9685 is only 12 bits but we are using the full 16 bit range
    low >>= 4;
    high = 0;
  }

  return pwm_driver_pca9685_set_pwm(&motor_kit->driver, pwm_channel, high, low);
}

static int update_coils(motor_kit_t *motor_kit, stepper_motor_t *stepper,
                        bool microstepping) {
  uint16_t duty_cycles[4] = {0, 0, 0, 0};
  uint8_t trailing_coil =
      positive_modulo((stepper->current_microstep / stepper->microsteps), 4);
  uint8_t leading_coil =
      (trailing_coil + 1) %
      4; // Always positive so no needed to use positive_modulo
  int microstep =
      positive_modulo(stepper->current_microstep, stepper->microsteps);
  duty_cycles[leading_coil] = stepper->curve[microstep];
  duty_cycles[trailing_coil] = stepper->curve[stepper->microsteps - microstep];

  /**
   * This ensure that double steps use full torque. read more about why here
   * https://github.com/adafruit/Adafruit_CircuitPython_Motor/blob/main/adafruit_motor/stepper.py#L159
   */
  if (!microstepping &&
      (duty_cycles[leading_coil] == duty_cycles[trailing_coil]) &&
      duty_cycles[leading_coil] > 0) {
    duty_cycles[leading_coil] = 0xFFFF;
    duty_cycles[trailing_coil] = 0xFFFF;
  }

  int res = EOK;
  res |= set_pwm_duty_cycle(motor_kit, stepper->a2, duty_cycles[0]);
  res |= set_pwm_duty_cycle(motor_kit, stepper->b1, duty_cycles[1]);
  res |= set_pwm_duty_cycle(motor_kit, stepper->a1, duty_cycles[2]);
  res |= set_pwm_duty_cycle(motor_kit, stepper->b2, duty_cycles[3]);
  return res;
}
