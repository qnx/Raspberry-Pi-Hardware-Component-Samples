# rpi_gpio

This folder contains a client API for interacting with
the GPIO resource manager.  The API was designed to be
similar and as easy to use as the GPIO Python API.

#### rpi_i2c

This folder contains a client API for interacting with
the I2C driver.  The API was designed to be
similar and as easy to use as the SMBus Python API.

Note that currently any app using this API needs to be
executed as root to access the I2C device driver.

#### rpi_spi

This folder contains a client API for interacting with
the SPI driver.  The API was designed to cover the
the different commands documented here: 

https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.spi_framework/topic/spi_ch_overview.html#SPI_POSIX

Note that currently any app using this API needs to be
executed as root to access the SPI device driver.

## Development Host Setup Resources

TBD

## How to obtain the RaspBerry PI 4 Quickstart Image

TBD

## Contributor Guidelines

TBD
