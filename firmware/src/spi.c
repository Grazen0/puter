#include "spi.h"
#include "puter.h"

void spi_write(const u8 byte)
{
    while (!SPI->ready) {
    }

    SPI->out = byte;
}

u8 spi_read(void)
{
    while (!SPI->ready) {
    }

    SPI->start_read = 1;

    while (!SPI->rdata_valid) {
    }

    return SPI->rdata;
}
