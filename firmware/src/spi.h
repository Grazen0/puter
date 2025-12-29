#ifndef FIRMWARE_SPI_H
#define FIRMWARE_SPI_H

#include "numeric.h"
#include <stddef.h>

void spi_set_freq(size_t freq);

void spi_write(u8 byte);

void spi_write_u32(u32 word);

u8 spi_read();

void spi_cs_enable();

void spi_cs_disable();

#endif
