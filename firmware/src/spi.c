#include "spi.h"
#include "numeric.h"
#include "puter.h"
#include <stddef.h>

static inline void spi_wait_ready()
{
    while (!SPI->ready) {
    }
}

void spi_cs_enable()
{
    spi_wait_ready();
    SPI->cs = 0;
}

void spi_cs_disable()
{
    spi_wait_ready();
    SPI->cs = 1;
}

void spi_set_freq(const size_t freq)
{
    spi_wait_ready();
    SPI->sclk_half_period = SYS_CLK_FREQ / (2 * freq);
}

u8 spi_transfer(const u8 write_data)
{
    spi_wait_ready();
    SPI->transfer = write_data;

    while (!SPI->rdata_valid) {
    }

    return SPI->rdata;
}

void spi_write(const u8 byte)
{
    spi_wait_ready();
    SPI->transfer = byte;
}

void spi_write_u32(u32 word)
{
    for (size_t i = 0; i < 4; ++i) {
        spi_write(word >> 24);
        word <<= 8;
    }
}
