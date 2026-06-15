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

#ifndef MOTOR_KIT_MOTOR_KIT_H
#define MOTOR_KIT_MOTOR_KIT_H

#include "motor_kit/dc_motor.h"
#include "motor_kit/stepper_motor.h"

#include <pca9685.h>

#define MOTOR_KIT_I2C_BUS_DEFAULT 1
#define MOTOR_KIT_I2C_ADDR_DEFAULT 0x60
#define MOTOR_KIT_PWM_FREQ_DEFAULT 1600

typedef struct motor_kit_s {
  /**
   * PWM driver needed to control the servos
   */
  pwm_driver_pca9685_t driver;

  /**
   * Stepper motors for the motor kit. Only valid once initalized.
   */
  stepper_motor_t stepper1; // Disables motor 1 and 2
  stepper_motor_t stepper2; // Disables motor 3 and 4

  /**
   * DC motor for the motor kit. Only valid once initalized.
   */
  dc_motor_t motor1;
  dc_motor_t motor2;
  dc_motor_t motor3;
  dc_motor_t motor4;

} motor_kit_t;

/**
 * @brief Initalized the motor kit.
 *
 * @note before use you still need to call motor_kit_stepper_init with the
 * motors you want to use.
 *
 * @param motor_kit motor_kit object to be initalized
 * @param i2c_bus Which i2c bus to use (default 1)
 * @param i2c_addr Which i2c addr to use (default 0x60)
 * @param freq What frequency should the PWM driver run at (default 2000)
 *
 * @return EOK on success, otherwise and error code
 */
int motor_kit_init(motor_kit_t *motor_kit, int i2c_bus, int i2c_addr, int freq);

/**
 * @brief Destroy motor kit and any configured devices
 *
 * @note The object should be considered invalid after this call. Any usage is
 * considered UB.
 * @param motor_kit Motor kit to destroy
 *
 */
void motor_kit_destroy(motor_kit_t *motor_kit);

/**
 * @brief Init a stepper motor owned by the provided motor kit.
 *
 * @note Must be called after motor_kit_init()
 *
 * @param motor_kit Initialized motor kit
 * @param stepper pointer to stepper motor owned by motor_kit.
 *
 * @return EOK on success, otherwise an error code.
 */
int motor_kit_stepper_init(motor_kit_t *motor_kit, stepper_motor_t *stepper);

/**
 * @brief Move the stepper motor one step forward
 *
 * @param motor_kit Motor Kit which owns the stepper
 * @param stepper  Stepper motor to move
 * @param dir  Direction (forward or backwards) which it should move
 * @param style  How the stepper should move default is SINGLE which will move
 *               it forward. For more information about how it moves see
 *               @stepper_style_t.
 *
 *
 * @return EOK on success, otherwise an error code.
 */
int motor_kit_stepper_step(motor_kit_t *motor_kit, stepper_motor_t *stepper,
                           stepper_direction_t dir, stepper_style_t style);

/**
 * @brief Release the stepper motor
 *
 * @param motor_kit Owner of the stepper
 * @param stepper  Stepper to be released
 *
 * @return EOK on success, otherwise an error code.
 */
int motor_kit_stepper_release(motor_kit_t *motor_kit, stepper_motor_t *stepper);

/**
 * @brief Init a DC motor owned by the provided motor kit.
 *
 * @note Must be called after motor_kit_init()
 *
 * @param motor_kit Initialized motor kit
 * @param motor pointer to DC motor owned by motor_kit.
 *
 * @return EOK on success, otherwise an error code.
 */
int motor_kit_dc_init(motor_kit_t *motor_kit, dc_motor_t *motor);

/**
 * @brief Turns on a motor to a given throttle from -100, 100
 *
 * @param motor_kit Owner of the DC motor
 * @param motor Motor to be moved
 * @param throttle Value -100, 100 of speed where 0 is a special breaking value.
 *                 If you want to release the motor see  @motor_kit_dc_release
 */
int motor_kit_dc_throttle(motor_kit_t *motor_kit, dc_motor_t *motor,
                          int8_t throttle);

/**
 * @brief Release the DC motor
 *
 * @param motor_kit Owner of the DC motor
 * @param motor  Motor to be released
 *
 * @return EOK on success, otherwise an error code.
 */
int motor_kit_dc_release(motor_kit_t *motor_kit, dc_motor_t *motor);

#endif // MOTOR_KIT_MOTOR_H
