#ifndef FIRMWARE_SPI_H
#define FIRMWARE_SPI_H

#include "numeric.h"
#include "puter.h"
#include <stddef.h>

static inline void spi_set_freq(const size_t freq)
{
    SPI->sclk_half_period = SYS_CLK_FREQ / (2 * freq);
}

void spi_write(u8 byte);

void spi_write_u32(u32 word);

u8 spi_read();

void spi_cs_enable();

void spi_cs_disable();

#endif
