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

#ifndef PWM_DRIVER_PCA9685_H_INCLUDED
#define PWM_DRIVER_PCA9685_H_INCLUDED

#include <stdint.h>

#define PCA9685_DEFAULT_I2C_BUS 1
#define PCA9685_DEFAULT_I2C_ADDR 0x40

typedef struct pwm_driver_pca9685_s {
    int i2c_bus;
    int i2c_addr;
} pwm_driver_pca9685_t;

/**
 * @brief Initializes PWM Driver object
 * 
 * @param driver Non NULL PWM driver.
 * @param i2c_bus Which I2C Bus is the driver attached to (default: PCA9685_DEFAULT_I2C_BUS)
 * @param i2c_addr Which I2C Address is the driver attached to (default PCA9685_DEFAULT_I2C_ADDR)
 * 
 * @return EOK on success, otherwise an error code.
 */
int pwm_driver_pca9685_init(pwm_driver_pca9685_t* driver, int i2c_bus, int i2c_addr);

/**
 * @brief Cleanup any resource associated with the driver
 * 
 * @note Usage of the driver after this call is considred undefined behaviour.
 * 
 * @param driver PWM driver which has previously been initialized with @pwm_driver_pca9685_init
 */
void pwm_driver_pca9685_destroy(pwm_driver_pca9685_t* driver);

/**
 * @brief Software reset the PCA9685 to it's initial state.
 * 
 * @param driver PWM driver which has previously been initialized with @pwm_driver_pca9685_init
 * 
 * @return EOK on success, otherwise an error code.
 */
int pwm_driver_pca9685_soft_reset(pwm_driver_pca9685_t* driver);

/**
 * @brief Set the PWM frequency of the PCA9685.
 * 
 * @param driver PWM driver which has previously been initialized with @pwm_driver_pca9685_init
 * @param freq The frequency in hz which the driver will be set to.
 *  
 * @return EOK on success, otherwise an error code.
 */
int pwm_driver_pca9685_set_frequency(pwm_driver_pca9685_t* driver, uint32_t freq);

/**
 * @brief Set a specific PWM pin high and low timing.
 * 
 * @param driver PWM driver which has previously been initialized with @pwm_driver_pca9685_init
 * @param pin Which pin to configure (0-15)
 * @param high How long should the pin be high
 * @param low How long should the pin be low
 */
int pwm_driver_pca9685_set_pwm(pwm_driver_pca9685_t* driver, uint8_t pin, int high, int low);

/**
 * @brief Set a specific PWM pin high and low timing.
 * 
 * @param driver PWM driver which has previously been initialized with @pwm_driver_pca9685_init
 * @param high How long should all the pins be high
 * @param low How long should all the pins be low
 */
int pwm_driver_pca9685_set_pwm_all(pwm_driver_pca9685_t* driver, int high, int low);


#endif // PWM_DRIVER_PCA9685_H_INCLUDED
