#include "sd_card.h"
#include "control.h"
#include "numeric.h"
#include "puter.h"
#include <stddef.h>
#include <stdio.h>

static inline void spi_set_freq(const size_t freq)
{
    SPI->sclk_half_period = SYS_CLK_FREQ / (2 * freq);
}

static inline void spi_wait_ready(void)
{
    while (!SPI->ready) {
    }
}

static inline void spi_write(const u8 byte)
{
    spi_wait_ready();
    SPI->out = byte;
}

static inline void spi_write_u32(u32 word)
{
    for (size_t i = 0; i < 4; ++i) {
        spi_write(word >> 24);
        word <<= 8;
    }
}

static inline u8 spi_read(void)
{
    spi_wait_ready();

    SPI->start_read = 1;

    while (!SPI->rdata_valid) {
    }

    return SPI->rdata;
}

static inline void spi_cs_enable(void)
{
    spi_wait_ready();
    SPI->cs = 0;
}

static inline void spi_cs_disable(void)
{
    spi_wait_ready();
    SPI->cs = 1;
}

typedef enum : u8 {
    SD_GO_IDLE = 0,
    SD_INIT = 1,
    SD_CHECK_V = 8,
    SD_STOP_READ = 12,
    SD_SET_BLOCKLEN = 16,
    SD_READ_SINGLE_BLOCK = 17,
    SD_READ_MULTI_BLOCK = 18,
    SD_WRITE_MULTI_BLOCK = 25,
    SD_ACMD_LEADING = 55,
    SD_READ_OCR = 58,
    SD_APP_INIT = 41,
} SdCmd;

static constexpr size_t CRC_TABLE_SIZE = 256;
u8 crc_table[CRC_TABLE_SIZE];

static inline u8 crc_add(const u8 crc, const u8 message_byte)
{
    return crc_table[(crc << 1) ^ message_byte];
}

// returns the CRC-7 for a message of "length" bytes
u8 get_crc(const u8 message[], const size_t length)
{
    u8 crc = 0;

    for (size_t i = 0; i < length; ++i)
        crc = crc_add(crc, message[i]);

    return crc;
}

static constexpr size_t SPI_INIT_FREQ = 100'000;
static constexpr size_t SPI_FREQ = 10'000'000;

static constexpr size_t RES_TIMEOUT_BYTES = SPI_INIT_FREQ / 8;
static constexpr u8 R1_NONE = 0xFF;

static u8 sd_read_r1(void)
{
    for (size_t i = 0; i < RES_TIMEOUT_BYTES; ++i) {
        const u8 res = spi_read();

        if ((res & 0x80) == 0)
            return res;
    }

    return R1_NONE;
}

static u32 sd_read_r3(void)
{
    u32 ocr = 0;

    for (size_t j = 0; j < 4; ++j) {
        const u8 byte = spi_read();
        ocr = (ocr << 8) | byte;
    }

    return ocr;
}

static constexpr u8 CMD_START_BITS = 0x40;

static u8 sd_send_cmd_r1(const SdCmd cmd, const u32 arg, const u8 crc)
{
    spi_cs_enable();

    spi_write(cmd | CMD_START_BITS);
    spi_write_u32(arg);
    spi_write(crc);

    const u8 r1 = sd_read_r1();
    spi_cs_disable();
    spi_write(0xFF);

    return r1;
}

typedef struct {
    u8 r1;
    u32 r3;
} R3Response;

typedef enum {
    ADDRMODE_BYTE,
    ADDRMODE_BLOCK,
} AddressMode;

static AddressMode address_mode;

static R3Response sd_send_cmd_r3(const SdCmd cmd, const u32 arg, const u8 crc)
{
    spi_cs_enable();

    spi_write(cmd | CMD_START_BITS);
    spi_write_u32(arg);
    spi_write(crc);

    R3Response res;
    res.r1 = sd_read_r1();
    if (res.r1 == R1_NONE) {
        spi_cs_disable();
        return res;
    }

    res.r3 = sd_read_r3();

    spi_cs_disable();
    return res;
}

SdInitResult sd_init(void)
{
    static constexpr size_t GO_IDLE_RETRIES = 10;

    static constexpr u32 GO_IDLE_ARG = 0x00000000;
    static constexpr u32 GO_IDLE_CRC = 0x95;

    static constexpr u32 CHECK_V_ARG = 0x000001AA;
    static constexpr u32 CHECK_V_CRC = 0x86;

    static constexpr u32 READ_OCR_ARG = 0x00000000;
    static constexpr u32 READ_OCR_CRC = 0x00;

    static constexpr u32 ACMD_LEADING_ARG = 0x00000000;
    static constexpr u32 ACMD_LEADING_CRC = 0x00;

    static constexpr u32 APP_INIT_ARG_V1 = 0x00000000;
    static constexpr u32 APP_INIT_CRC_V1 = 0x00;

    static constexpr u32 APP_INIT_ARG_V2 = 0x40000000;
    static constexpr u32 APP_INIT_CRC_V2 = 0x00;

    static constexpr u32 INIT_ARG = 0x00000000;
    static constexpr u32 INIT_CRC = 0x00;

    static constexpr u32 SET_BLOCKLEN_ARG = 0x00000200;
    static constexpr u32 SET_BLOCKLEN_CRC = 0x00;

    spi_set_freq(SPI_INIT_FREQ);

    // Wait for 80 cycles
    for (size_t i = 0; i < 10; ++i)
        spi_write(0xFF);

    u8 goidle_res = R1_NONE;

    for (size_t i = 0; i < GO_IDLE_RETRIES; ++i) {
        goidle_res = sd_send_cmd_r1(SD_GO_IDLE, GO_IDLE_ARG, GO_IDLE_CRC);

        if (goidle_res == 0x01)
            break;
    }

    if (goidle_res != 0x01)
        return SD_INIT_ERR_NOIDLE;
    else
        goto check_voltage;

check_voltage:
    const u8 checkv_res = sd_send_cmd_r1(SD_CHECK_V, CHECK_V_ARG, CHECK_V_CRC);

    if (checkv_res == 0x01)
        goto app_init_v2;
    else {
        goto app_init_v1;
    }

    // TODO: add timeout
app_init_v2:
    const u8 acmdlead_res_v2 = sd_send_cmd_r1(SD_ACMD_LEADING, ACMD_LEADING_ARG, ACMD_LEADING_CRC);
    PANIC_IF(acmdlead_res_v2 == R1_NONE);

    const u8 appinit_v2_res = sd_send_cmd_r1(SD_APP_INIT, APP_INIT_ARG_V2, APP_INIT_CRC_V2);

    if (appinit_v2_res == 0x00)
        goto read_ocr;
    else
        goto app_init_v2;

read_ocr:
    const R3Response readocr_res = sd_send_cmd_r3(SD_READ_OCR, READ_OCR_ARG, READ_OCR_CRC);

    if (readocr_res.r1 == 0x00)
        goto receive_ocr;
    else {
        address_mode = ADDRMODE_BYTE;
        goto set_blocklen; // SD v2 (byte address)
    }

receive_ocr:
    printf("Card OCR: %08X\n", readocr_res.r3);
    goto high_capacity;

high_capacity:
    if ((readocr_res.r3 & 0x4000'0000) != 0) {
        address_mode = ADDRMODE_BLOCK;
        goto inc_sclk_speed; // SD v2 (block address)
    } else {
        goto set_blocklen; // SD v2 (byte address)
    }

    // TODO: add timeout
app_init_v1:
    const u8 acmdlead_res_v1 = sd_send_cmd_r1(SD_ACMD_LEADING, ACMD_LEADING_ARG, ACMD_LEADING_CRC);
    PANIC_IF(acmdlead_res_v1 == R1_NONE);

    const u8 appinit_v1_res = sd_send_cmd_r1(SD_APP_INIT, APP_INIT_ARG_V1, APP_INIT_CRC_V1);

    if (appinit_v1_res == R1_NONE)
        goto init_mmc;
    else if (appinit_v1_res == 0x00)
        goto set_blocklen; // SD v1
    else {
        // printf("app init v1: %02X\n", appinit_v1_res);
        goto app_init_v1;
    }

    // TODO: add timeout
init_mmc:
    const u8 init_res = sd_send_cmd_r1(SD_INIT, INIT_ARG, INIT_CRC);

    if (init_res == R1_NONE)
        return SD_INIT_ERR_NOINIT;
    else if (init_res == 0x00) // MMC v3
        goto set_blocklen;
    else
        goto init_mmc;

set_blocklen:
    const u8 setblocklen_res = sd_send_cmd_r1(SD_SET_BLOCKLEN, SET_BLOCKLEN_ARG, SET_BLOCKLEN_CRC);

    if (setblocklen_res == 0x00)
        goto inc_sclk_speed;
    else
        return SD_INIT_ERR_NOSETBLOCKLEN;

inc_sclk_speed:
    spi_set_freq(SPI_FREQ);
    return SD_INIT_OK;
}

void sd_read_block(const u32 block_addr)
{
    static constexpr u32 READ_SINGLE_BLOCK_CRC = 0x00;

    static constexpr u8 START_TOKEN = 0xFE;
    static constexpr size_t BLOCK_SIZE = 512;

    static constexpr size_t MAX_READ_ATTEMPTS = 3000;

    spi_write(0xFF);
    spi_cs_enable();
    spi_write(0xFF);

    spi_write(SD_READ_SINGLE_BLOCK | CMD_START_BITS);
    spi_write_u32(block_addr);
    spi_write(READ_SINGLE_BLOCK_CRC);

    const u8 res = sd_read_r1();

    if (res != 0x00) {
        printf("could not read block (res = %02X)\n", res);
        for (size_t i = 0; i < 15; ++i)
            printf("%02X ", spi_read());

        printf("\n");
        goto cleanup;
    }

    u8 read;

    for (size_t i = 0; i < MAX_READ_ATTEMPTS; ++i) {
        read = spi_read();
        if (read != 0xFF)
            break;
    }

    if (read != START_TOKEN) {
        printf("read timed out (last read = %02X)\n", read);
        goto cleanup;
    }

    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        const u8 byte = spi_read();
        printf("%02X ", byte);
        fflush(nullptr);
    }

    // read crc
    const u8 crc_hi = spi_read();
    const u8 crc_lo = spi_read();
    const u16 crc = ((u16)crc_hi << 8) | crc_lo;

    printf("\n");
    printf("crc: %04X\n", crc);

cleanup:
    spi_write(0xFF);
    spi_cs_disable();
    spi_write(0xFF);
}

const char *sd_init_result_str(const SdInitResult init_result)
{
    switch (init_result) {
    case SD_INIT_OK:
        return "ok";
    case SD_INIT_ERR_NOIDLE:
        return "GO_IDLE timed out or did not respond correctly";
    case SD_INIT_ERR_NOOCR:
        return "could not read OCR";
    case SD_INIT_ERR_NOAPPINIT:
        return "APP_INIT did not respond correctly";
    case SD_INIT_ERR_NOINIT:
        return "INIT did not respond correctly";
    case SD_INIT_ERR_NOSETBLOCKLEN:
        return "SET_BLOCKLEN did not respond correctly";
    default:
        return "unknown init result";
    }
}
