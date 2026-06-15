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

#include <motor_kit/motor_kit.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define STEP_SLEEP_US 100000 // (0.01ms)

int main(int argc, char *argv[]) {
  int freq = MOTOR_KIT_PWM_FREQ_DEFAULT;
  int bus = MOTOR_KIT_I2C_BUS_DEFAULT;
  int addr = MOTOR_KIT_I2C_ADDR_DEFAULT;

  /**
   * Before using the motor kit we have to initialize it to gain access to the
   * DC over I2C.
   *
   * If you require more than 4 DC motors you are also able to stack
   * multiple hats to gain access to more interfaces. If multiple hats are used
   * they must all be initalized independently using their unique I2C address.
   *
   * For more information about how to stack the hats see:
   * https://learn.adafruit.com/adafruit-dc-and-stepper-motor-hat-for-raspberry-pi/stacking-hats
   */
  motor_kit_t motor_kit;
  int res = motor_kit_init(&motor_kit, bus, addr, freq);
  if (res != EOK) {
    printf("Failed to init motor_kit\n");
    return EXIT_FAILURE;
  }

   /**
   * In this sample we will make use of just motor 3 but to use multiple DC
   * motors repeat this process using motor_kit.motor2 or (motor1, motor4) instead of
   * motor_kit.motor1.
   * 
   * @note If stepper 1 is used motor1 and 2 cannot be initalized.
   */
  dc_motor_t *motor = &motor_kit.motor3;
  res = motor_kit_dc_init(&motor_kit, motor);
  if (res != EOK) {
    printf("Failed to init motor\n");
    return EXIT_FAILURE;
  }

  printf("Spinning forwards\n");
  for (float throttle = 0.25; throttle < 0.4; throttle += 0.01) {
    motor_kit_dc_throttle(&motor_kit, motor, throttle * 100);
    usleep(STEP_SLEEP_US);
  }

  printf("Spinning backwards\n");
  for (float throttle = 0.25; throttle < 0.4; throttle += 0.01) {
    motor_kit_dc_throttle(&motor_kit, motor, -100 + (throttle * 100));
    usleep(STEP_SLEEP_US);
  }

  printf("Releasing motor\n");
  motor_kit_dc_release(&motor_kit, motor);
  motor_kit_destroy(&motor_kit);
  return 0;
}
