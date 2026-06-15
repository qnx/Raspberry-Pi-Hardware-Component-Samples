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

#define STEP_SLEEP_NS 50000 // (0.05ms)

int main(int argc, char *argv[]) {
  int freq = MOTOR_KIT_PWM_FREQ_DEFAULT;
  int bus = MOTOR_KIT_I2C_BUS_DEFAULT;
  int addr = MOTOR_KIT_I2C_ADDR_DEFAULT;

  /**
   * Before using the motor kit we have to initialize it to gain access to the
   * steppers over I2C.
   *
   * If you require more than 2 stepper motors you are also able to stack
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
   * In this sample we will make use of just stepper 1 but to use both stepper
   * motors repeat this process using motor_kit.stepper2 instead of
   * motor_kit.stepper1.
   *
   * @note If stepper 1 is used motor1 and 2 cannot be initalized.
   */
  stepper_motor_t *stepper = &motor_kit.stepper1;
  res = motor_kit_stepper_init(&motor_kit, stepper);
  if (res != EOK) {
    printf("Failed to init stepper1\n");
    return EXIT_FAILURE;
  }

  /**
   * To move the stepper simply call motor_kit_stepper_step with the direction
   * you want it to spin keeping in mind normally 200 steps is considered a full
   * rotation.
   */
  for (int i = 0; i < 2; i++) {
    stepper_direction_t dir =
        ((i % 2) == 0) ? STEPPER_DIR_FORWARD : STEPPER_DIR_BACKWARDS;
    for (int i = 0; i < 200; i++) {
      motor_kit_stepper_step(&motor_kit, stepper, dir, STEPPER_STYLE_SINGLE);

      /**
       * Between steps we need to sleep to ensure that the shaft physically
       * moves to the next location before changing energizing the next coils.
       *
       * One important note here is we are using nanospin_us instead of
       * nanosleep to ensure we only sleep for the amount of time we specify and
       * not more. This is because nanosleep is driven by the system clock tick
       * rather than spinning in place meaning on average you will sleep for
       * 0.5ms rather than our specified 0.05ms. To read more about why this
       * happens see
       * https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/n/nanosleep.html.
       */
      nanospin_ns(STEP_SLEEP_NS);
    }
  }

  /**
   * When not using the stepper it is a good idea to release it if it doesn't
   * need to be held to increase the lifespan and lower the power draw of the
   * stepper. To continue using the stepper ofter release simply call
   * motor_kit_stepper_step() in the direction you want to go.
   */
  motor_kit_stepper_release(&motor_kit, &motor_kit.stepper1);

  motor_kit_destroy(&motor_kit);
  return 0;
}
