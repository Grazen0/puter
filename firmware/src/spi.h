#ifndef FIRMWARE_SPI_H
#define FIRMWARE_SPI_H

#include "numeric.h"

void spi_write(u8 byte);

u8 spi_read(void);

#endif
