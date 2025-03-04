/*
 * Copyright (c) 2024, BlackBerry Limited. All rights reserved.
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

#ifndef RPI_I2C_API_H
#define RPI_I2C_API_H

#include <hw/i2c.h>

/* Return codes for client API */
#define I2C_SUCCESS 0
#define I2C_ERROR_NOT_CONNECTED -1
#define I2C_ERROR_ALLOC_FAILED -2
#define I2C_ERROR_OPERATION_FAILED -3
#define I2C_ERROR_CLEANING_UP -4

/* the I2C receive data message structure (allocate extra spaces for data bytes) */
struct i2c_recv_data_msg_t
{
    i2c_sendrecv_t hdr;
    uint8_t bytes[0];
};

/* the I2C send data message structure (allocate extra spaces for data bytes) */
struct i2c_send_data_msg_t
{
    i2c_send_t hdr;
    uint8_t bytes[0];
};

/**
 * Reads one byte from a specific address and a specific register
 *
 * @param    bus_number      I2C bus number
 * @param    i2c_address     I2C address
 * @param    register_val    register value
 * @param    value           value of register byte read (output)
 *
 * @returns  I2C_SUCCESS                 on success,
 *           I2C_ERROR_NOT_CONNECTED     if the i2C device is not available to connect to
 *           I2C_ERROR_ALLOC_FAILED      device control packet allocation failed
 *           I2C_ERROR_OPERATION_FAILED  I2C operation failed
 */
int smbus_read_byte_data(unsigned bus_number, uint8_t i2c_address, uint8_t register_val, uint8_t *value);

/**
 * Reads a block of bytes specified at a specific address with a specific register
 *
 * @param    bus_number      I2C bus number
 * @param    i2c_address     I2C address
 * @param    register_val    register value
 * @param    block_buffer    pointer to buffer to store read data
 * @param    block_size      size of buffer
 *
 * @returns  I2C_SUCCESS                 on success,
 *           I2C_ERROR_NOT_CONNECTED     if the i2C device is not available to connect to
 *           I2C_ERROR_ALLOC_FAILED      device control packet allocation failed
 *           I2C_ERROR_OPERATION_FAILED  I2C operation failed
 */
int smbus_read_block_data(unsigned bus_number, uint8_t i2c_address, uint8_t register_val, uint8_t *block_buffer, uint8_t block_size);

/**
 * Write any uint8_t value at a specific address with a specific register
 *
 * @param    bus_number      I2C bus number
 * @param    i2c_address     I2C address
 * @param    register_val    register value
 * @param    value           value of register byte to write
 *
 * @returns  I2C_SUCCESS                 on success,
 *           I2C_ERROR_NOT_CONNECTED     if the i2C device is not available to connect to
 *           I2C_ERROR_ALLOC_FAILED      device control packet allocation failed
 *           I2C_ERROR_OPERATION_FAILED  I2C operation failed
 */
int smbus_write_byte_data(unsigned bus_number, uint8_t i2c_address, uint8_t register_val, const uint8_t value);

/**
 * Writes a block of bytes specified at a specific address with a specific register
 *
 * @param    bus_number      I2C bus number
 * @param    i2c_address     I2C address
 * @param    register_val    register value
 * @param    block_buffer    pointer to buffer for data to write
 * @param    block_size      size of buffer
 *
 * @returns  I2C_SUCCESS                 on success,
 *           I2C_ERROR_NOT_CONNECTED     if the i2C device is not available to connect to
 *           I2C_ERROR_ALLOC_FAILED      device control packet allocation failed
 *           I2C_ERROR_OPERATION_FAILED  I2C operation failed
 */
int smbus_write_block_data(unsigned bus_number, uint8_t i2c_address, uint8_t register_val, uint8_t *block_buffer, uint8_t block_size);

/**
 * Clean up I2C API resources
 *
 * @param    bus_number      I2C bus number
 *
 * @returns  I2C_SUCCESS                 on success
 *           I2C_ERROR_NOT_CONNECTED     if resource manager not available to connect to
 *           I2C_ERROR_CLEANING_UP       if there is a failure disconnecting from the resource manager
 */
int smbus_cleanup(unsigned bus_number);

#endif
