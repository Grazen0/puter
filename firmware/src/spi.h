#ifndef FIRMWARE_SPI_H
#define FIRMWARE_SPI_H

#include "numeric.h"
#include "puter.h"
#include <stddef.h>

void spi_cs_enable();

void spi_cs_disable();

void spi_set_hperiod(size_t half_period);

u8 spi_transfer(u8 write_data);

void spi_write(u8 byte);

void spi_write_u32(u32 word);

static inline u8 spi_read()
{
    return spi_transfer(0xFF);
}

#endif
