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

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "public/rpi_i2c.h"

#define I2C_FILENAME_FORMAT "/dev/i2c%d"
#define MAX_I2C_BUSES       10 // TODO: Is this correct?
#define MIN_READ_BYTES      1
#define MIN_WRITE_BYTES     2

static int smbus_fd[MAX_I2C_BUSES] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

// Mutex protecting the smbus device file descriptors
static pthread_mutex_t smbus_fd_mutex;

/* Open the i2C bus device */
static
int open_smbus_fd(unsigned bus_number)
{
    char smbus_device_name[15] = { 0 };

    pthread_mutex_lock(&smbus_fd_mutex);

    sprintf(smbus_device_name, I2C_FILENAME_FORMAT, bus_number);

    if (smbus_fd[bus_number] == -1)
    {
        smbus_fd[bus_number] = open(smbus_device_name, O_RDWR);
        if (smbus_fd[bus_number] < 0)
        {
            perror("open");
            return I2C_ERROR_NOT_CONNECTED;
        }
    }

    pthread_mutex_unlock(&smbus_fd_mutex);

    return I2C_SUCCESS;
}

/* Close the i2C bus device */
static
int close_smbus_fd(unsigned bus_number)
{
    pthread_mutex_lock(&smbus_fd_mutex);

    if (smbus_fd[bus_number] != -1)
    {
        int err = close(smbus_fd[bus_number]);
        if (err != EOK)
        {
            perror("close");
            return I2C_ERROR_NOT_CONNECTED;
        }
        smbus_fd[bus_number] = -1;
    }

    pthread_mutex_unlock(&smbus_fd_mutex);

    return I2C_SUCCESS;
}

int smbus_read_byte_data(unsigned bus_number, uint8_t i2c_address, uint8_t register_val, uint8_t *value)
{
    // error variable
    int err;

    if (open_smbus_fd(bus_number))
    {
        perror("open_smbus_fd");
        return I2C_ERROR_NOT_CONNECTED;
    }

    // Allocate memory for the message
    struct i2c_recv_data_msg_t *msg = NULL;
    msg = malloc(sizeof(struct i2c_recv_data_msg_t) + MIN_READ_BYTES); // allocate enough memory for both the calling information and received data
    if (!msg)
    {
        perror("alloc failed");
        return I2C_ERROR_ALLOC_FAILED;
    }

    // Assign which register
    msg->bytes[0] = register_val;

    // Assign the I2C device and format of message
    msg->hdr.slave.addr = i2c_address;
    msg->hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    msg->hdr.send_len = 1;
    msg->hdr.recv_len = 1;
    msg->hdr.stop = 1;

    // Send the I2C message
    int status; // status information about the devctl() call
    err = devctl(smbus_fd[bus_number], DCMD_I2C_SENDRECV, msg, sizeof(*msg) + 1, (&status));
    if (err != EOK)
    {
        free(msg);
        perror("devctl");
        return I2C_ERROR_OPERATION_FAILED;
    }

    // return the read data
    *value = msg->bytes[0];

    // Free allocated message
    free(msg);

    return I2C_SUCCESS;
}

int smbus_read_block_data(unsigned bus_number, uint8_t i2c_address, uint8_t register_val, uint8_t *block_buffer, uint8_t block_size)
{
    int err;

    if (open_smbus_fd(bus_number))
    {
        perror("open_smbus_fd");
        return I2C_ERROR_NOT_CONNECTED;
    }

    if (block_size < MIN_READ_BYTES) {
        block_size = MIN_READ_BYTES;
    }

    // Allocate memory for the message
    struct i2c_recv_data_msg_t *msg = NULL;
    msg = malloc(sizeof(struct i2c_recv_data_msg_t) + block_size); // allocate enough memory for both the calling information and received data
    if (!msg)
    {
        perror("alloc failed");
        return I2C_ERROR_ALLOC_FAILED;
    }

    // Assign which register
    msg->bytes[0] = register_val;

    // Assign the I2C device and format of message
    msg->hdr.slave.addr = i2c_address;
    msg->hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    msg->hdr.send_len = 1;
    msg->hdr.recv_len = block_size;
    msg->hdr.stop = 1;

    // Send the I2C message
    int status; // status information about the devctl() call
    err = devctl(smbus_fd[bus_number], DCMD_I2C_SENDRECV, msg, sizeof(struct i2c_recv_data_msg_t) + block_size, (&status));
    if (err != EOK)
    {
        free(msg);
        perror("devctl");
        return I2C_ERROR_OPERATION_FAILED;
    }

    // Save the read data
    for (int i = 0; i < block_size; i++)
    {
        block_buffer[i] = msg->bytes[i];
    }

    // Free allocated message
    free(msg);

    return I2C_SUCCESS;
}

int smbus_write_byte_data(unsigned bus_number, uint8_t i2c_address, uint8_t register_val, const uint8_t value)
{
    int err;

    if (open_smbus_fd(bus_number))
    {
        perror("open_smbus_fd");
        return I2C_ERROR_NOT_CONNECTED;
    }

    // Allocate memory for the message
    struct i2c_send_data_msg_t *msg = NULL;
    msg = malloc(sizeof(struct i2c_send_data_msg_t) + MIN_WRITE_BYTES); // allocate enough memory for both the calling information and received data
    if (!msg)
    {
        perror("alloc failed");
        return I2C_ERROR_ALLOC_FAILED;
    }

    // Assign which register gets what value
    msg->bytes[0] = register_val;
    msg->bytes[1] = value;

    // Assign the I2C device and format of message
    msg->hdr.slave.addr = i2c_address;
    msg->hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    msg->hdr.len = MIN_WRITE_BYTES;
    msg->hdr.stop = 1;

    // Send the I2C message
    err = devctl(smbus_fd[bus_number], DCMD_I2C_SEND, msg, sizeof(struct i2c_send_data_msg_t) + MIN_WRITE_BYTES, NULL);
    if (err != EOK)
    {
        free(msg);
        perror("error with devctl");
        return I2C_ERROR_OPERATION_FAILED;
    }

    // Free allocated memory
    free(msg);

    return I2C_SUCCESS;
}

int smbus_write_block_data(unsigned bus_number, uint8_t i2c_address, uint8_t register_val, uint8_t *block_buffer, uint8_t block_size)
{
    int err;

    if (open_smbus_fd(bus_number))
    {
        perror("open_smbus_fd");
        return I2C_ERROR_NOT_CONNECTED;
    }

    if (block_size < MIN_WRITE_BYTES) {
        block_size = MIN_WRITE_BYTES;
    }

    // Allocate memory for the message
    struct i2c_send_data_msg_t *msg = NULL;
    msg = malloc(sizeof(struct i2c_send_data_msg_t) + block_size + 1); // allocate enough memory for both the calling information and received data
    if (!msg)
    {
        perror("alloc failed");
        return I2C_ERROR_ALLOC_FAILED;
    }

    // Assign which register gets what value
    msg->bytes[0] = register_val;
    // Add the write data
    for (int i = 0; i < block_size; i++)
    {
        msg->bytes[1 + i] = block_buffer[i];
    }

    // Assign the I2C device and format of message
    msg->hdr.slave.addr = i2c_address;
    msg->hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    msg->hdr.len = block_size + 1;
    msg->hdr.stop = 1;

    // Send the I2C message
    int status; // status information about the devctl() call
    err = devctl(smbus_fd[bus_number], DCMD_I2C_SEND, msg, sizeof(struct i2c_send_data_msg_t) + block_size + 1, (&status));
    if (err != EOK)
    {
        free(msg);
        perror("error with devctl");
        return I2C_ERROR_OPERATION_FAILED;
    }

    // Free allocated message
    free(msg);

    return I2C_SUCCESS;
}

int smbus_cleanup(unsigned bus_number)
{
    int status;

    status = close_smbus_fd(bus_number);
    if (status)
    {
        perror("close_smbus_fd");
        status = I2C_ERROR_CLEANING_UP;
    }

    return I2C_SUCCESS;
}
