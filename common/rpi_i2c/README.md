# librpi_i2c

This folder contains a client API, implemented as shared
library, for interacting with the I2C driver.  The API was
designed to be similar and as easy to use as the SMBus Python
module.

Note that currently any app using this client API needs to be
executed as root to access the I2C device driver.

## smbus_read_byte_data

Reads one byte from a specific address and a specific register

## smbus_read_block_data

Reads a block of bytes specified at a specific address with a specific register

## smbus_write_byte_data

Write any uint8_t value at a specific address with a specific register

## smbus_write_block_data

Writes a block of bytes specified at a specific address with a specific register

## smbus_cleanup

Clean up I2C API resources

---
See [rpi_i2c.h](rpi_i2c.h) for more details.
