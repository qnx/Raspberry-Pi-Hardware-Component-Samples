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

// Based on Adafruit implementation
// (https://github.com/adafruit/Adafruit_CircuitPython_PCA9685/blob/main/adafruit_pca9685.py)
/**
 * Copyright (c) 2016 Adafruit Industries
 * Author: Tony DiCola
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 **/

#include "pca9685.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <rpi_i2c.h>

#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_PRESCALE 0xFE
#define PCA9685_SOFT_RESET 0x06
#define PCA9685_LED0_ON_L 0x06
#define PCA9685_ALL_LED_ON_L 0xFA

// Reset bit flags
#define PCA9685_SLEEP_BIT 0x10
#define PCA9685_ALLCALL_BIT 0x01
#define PCA9685_OUTDRV_BIT 0x04

int pwm_driver_pca9685_init(pwm_driver_pca9685_t *driver, int i2c_bus,
                            int i2c_addr) {
  if (driver == NULL) {
    return EINVAL;
  }

  driver->i2c_addr = i2c_addr;
  driver->i2c_bus = i2c_bus;

  /**
   * Before initializing the controller make sure that all PWM pins are set to
   * 0,0 Depending on the servo this will cause the gear to release
   **/
  int ret = pwm_driver_pca9685_set_pwm_all(driver, 0, 0);
  if (ret != I2C_SUCCESS) {
    return EIO;
  }

  /**
   * Initialize the Controller with ALLCALL enabled and sleep disabled
   */
  ret |= smbus_write_byte_data(driver->i2c_bus, driver->i2c_addr, PCA9685_MODE2,
                               (PCA9685_OUTDRV_BIT | 0x20));
  ret |= smbus_write_byte_data(driver->i2c_bus, driver->i2c_addr, PCA9685_MODE1,
                               PCA9685_ALLCALL_BIT);
  usleep(5000); // wait for oscillator

  uint8_t mode1_val = 0;
  ret |= smbus_read_byte_data(driver->i2c_bus, driver->i2c_addr, PCA9685_MODE1,
                              &mode1_val);
  mode1_val = mode1_val & ~PCA9685_SLEEP_BIT;

  ret |= smbus_write_byte_data(driver->i2c_bus, driver->i2c_addr, PCA9685_MODE1,
                               mode1_val);
  usleep(5000); // wait for oscillator

  /**
   * Check if any of the operations failed if so we must cleanup the i2c
   * interface before returning.
   */
  if (ret != EOK) {
    smbus_cleanup(i2c_bus);
    driver->i2c_bus = -1;
    ret = EIO;
  }

  return ret;
}

void pwm_driver_pca9685_destroy(pwm_driver_pca9685_t *driver) {
  if (driver == NULL) {
    return;
  }

  // Set the driver into a good know state before shutdown
  pwm_driver_pca9685_set_pwm_all(driver, 0, 0);
  pwm_driver_pca9685_soft_reset(driver);

  smbus_cleanup(driver->i2c_bus);
}

int pwm_driver_pca9685_soft_reset(pwm_driver_pca9685_t *driver) {
  if (driver == NULL) {
    return EINVAL;
  }
  return smbus_write_byte_data(driver->i2c_bus, driver->i2c_addr, PCA9685_SOFT_RESET, 0x00);
}

int pwm_driver_pca9685_set_frequency(pwm_driver_pca9685_t *driver,
                                     uint32_t freq) {
  if (driver == NULL) {
    return EINVAL;
  }
  float reference_clk_speed = 25000000.0; // 25MHz
  uint8_t prescale = (int)(reference_clk_speed / 4096.0 / freq + 0.5) - 1;

  int ret = EOK;
  uint8_t old_mode = 0;
  ret |= smbus_read_byte_data(driver->i2c_bus, driver->i2c_addr, PCA9685_MODE1,
                              &old_mode);
  uint8_t new_mode = (old_mode & 0x7F) | PCA9685_SLEEP_BIT; // sleep
  ret |= smbus_write_byte_data(driver->i2c_bus, driver->i2c_addr, PCA9685_MODE1,
                               new_mode);
  ret |= smbus_write_byte_data(driver->i2c_bus, driver->i2c_addr,
                               PCA9685_PRESCALE, prescale);
  ret |= smbus_write_byte_data(driver->i2c_bus, driver->i2c_addr, PCA9685_MODE1,
                               old_mode);

  usleep(5000);
  ret |= smbus_write_byte_data(driver->i2c_bus, driver->i2c_addr, PCA9685_MODE1,
                               (old_mode | 0xA0));

  return ret;
}

int pwm_driver_pca9685_set_pwm(pwm_driver_pca9685_t *driver, uint8_t pin,
                               int high, int low) {
  if (driver == NULL || pin > 15) {
    return EINVAL;
  }
  uint8_t buffer[4] = {high & 0xFF, high >> 8, low & 0xFF, low >> 8};
  if (smbus_write_block_data(driver->i2c_bus, driver->i2c_addr,
                             PCA9685_LED0_ON_L + 4 * pin, buffer,
                             4) != I2C_SUCCESS) {
    return EIO;
  }
  return EOK;
}

int pwm_driver_pca9685_set_pwm_all(pwm_driver_pca9685_t *driver, int high,
                                   int low) {
  if (driver == NULL) {
    return EINVAL;
  }
  uint8_t buffer[4] = {high & 0xFF, high >> 8, low & 0xFF, low >> 8};
  if (smbus_write_block_data(driver->i2c_bus, driver->i2c_addr,
                             PCA9685_ALL_LED_ON_L, buffer, 4) != I2C_SUCCESS) {
    return EIO;
  }
  return EOK;
}
