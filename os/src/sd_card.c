#include "sd_card.h"
#include "numeric.h"
#include "puter.h"
#include "spi.h"
#include <stddef.h>

#define FREQ_TO_HALF_PERIOD(FREQ) (SYS_CLK_FREQ / (2 * (FREQ)))

typedef enum : u8 {
    SdCmd_GoIdle = 0,
    SdCmd_Init = 1,
    SdCmd_CheckV = 8,
    SdCmd_StopRead = 12,
    SdCmd_SetBlockLen = 16,
    SdCmd_ReadSingleBlock = 17,
    SdCMd_ReadMultiBlock = 18,
    SdCmd_WriteMultiBlock = 25,
    SdCmd_AcmdLeading = 55,
    SdCmd_ReadOcr = 58,
    SdCmd_AppInit = 41,
} SdCmd;

static constexpr size_t SPI_INIT_FREQ = 100'000U;
static constexpr size_t SPI_FREQ = 12'500'000U;

static_assert((SYS_CLK_FREQ % (2 * SPI_INIT_FREQ)) == 0);
static_assert((SYS_CLK_FREQ % (2 * SPI_FREQ)) == 0);

static constexpr size_t RES_TIMEOUT_BYTES = 10'000U;
static constexpr u8 R1_NONE = 0xFF;

static u8 sd_read_r1()
{
    for (size_t i = 0; i < RES_TIMEOUT_BYTES; ++i) {
        const u8 res = spi_read();

        if ((res & 0x80) == 0)
            return res;
    }

    return R1_NONE;
}

static u32 sd_read_r3()
{
    u32 r3 = 0;

    for (size_t j = 0; j < 4; ++j) {
        const u8 byte = spi_read();
        r3 = (r3 << 8) | byte;
    }

    return r3;
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

typedef enum : u8 {
    AddressMode_Byte,
    AddressMode_Block,
} AddressMode;

typedef struct {
    AddressMode address_mode;
    bool ready;
    volatile bool dma_finished;
} SdCardContext;

static SdCardContext ctx;

static R3Response sd_send_cmd_r3(const SdCmd cmd, const u32 arg, const u8 crc)
{
    spi_cs_enable();

    spi_write(cmd | CMD_START_BITS);
    spi_write_u32(arg);
    spi_write(crc);

    R3Response res;
    res.r1 = sd_read_r1();
    if (res.r1 == R1_NONE)
        goto cleanup;

    res.r3 = sd_read_r3();

cleanup:
    spi_cs_disable();
    return res;
}

SdInitResult sd_init()
{
    constexpr size_t GO_IDLE_RETRIES = 10;

    constexpr u32 GO_IDLE_ARG = 0x00000000;
    constexpr u8 GO_IDLE_CRC = 0x95;

    constexpr u32 CHECK_V_ARG = 0x000001AA;
    constexpr u8 CHECK_V_CRC = 0x86;

    constexpr u32 READ_OCR_ARG = 0x00000000;
    constexpr u8 READ_OCR_CRC = 0x00;

    constexpr u32 ACMD_LEADING_ARG = 0x00000000;
    constexpr u8 ACMD_LEADING_CRC = 0x00;

    constexpr u32 APP_INIT_ARG_V1 = 0x00000000;
    constexpr u8 APP_INIT_CRC_V1 = 0x00;

    constexpr u32 APP_INIT_ARG_V2 = 0x40000000;
    constexpr u8 APP_INIT_CRC_V2 = 0x00;

    constexpr u32 INIT_ARG = 0x00000000;
    constexpr u8 INIT_CRC = 0x00;

    constexpr u32 SET_BLOCKLEN_ARG = 0x00000200;
    constexpr u8 SET_BLOCKLEN_CRC = 0x00;

    ctx = (SdCardContext){
        .address_mode =
            AddressMode_Byte, // Most SD card types use byte addressing
        .ready = false,
        .dma_finished = false,
    };

    spi_set_hperiod(FREQ_TO_HALF_PERIOD(SPI_INIT_FREQ));

    // Wait for 80 cycles
    for (size_t i = 0; i < 10; ++i)
        spi_write(0xFF);

    u8 goidle_res = R1_NONE;

    for (size_t i = 0; i < GO_IDLE_RETRIES; ++i) {
        goidle_res = sd_send_cmd_r1(SdCmd_GoIdle, GO_IDLE_ARG, GO_IDLE_CRC);

        if (goidle_res == 0x01)
            break;
    }

    if (goidle_res == 0x01)
        goto check_voltage;
    else
        return SdInitResult_ErrGoIdleTimedOut;

check_voltage:
    const u8 checkv_res =
        sd_send_cmd_r1(SdCmd_CheckV, CHECK_V_ARG, CHECK_V_CRC);

    if (checkv_res == 0x01)
        goto app_init_v2;
    else
        goto app_init_v1;

    // TODO: add timeout
app_init_v2:
    const u8 acmdlead_res_v2 =
        sd_send_cmd_r1(SdCmd_AcmdLeading, ACMD_LEADING_ARG, ACMD_LEADING_CRC);

    if (acmdlead_res_v2 == R1_NONE)
        return SdInitResult_ErrNoAcmdLeading;

    const u8 appinit_v2_res =
        sd_send_cmd_r1(SdCmd_AppInit, APP_INIT_ARG_V2, APP_INIT_CRC_V2);

    if (appinit_v2_res == 0x00)
        goto read_ocr;
    else
        goto app_init_v2;

read_ocr:
    const R3Response readocr_res =
        sd_send_cmd_r3(SdCmd_ReadOcr, READ_OCR_ARG, READ_OCR_CRC);

    if (readocr_res.r1 == 0x00)
        goto receive_ocr;
    else
        goto set_blocklen; // SD v2 (byte address)

receive_ocr:
    goto high_capacity;

high_capacity:
    if ((readocr_res.r3 & 0x4000'0000) != 0) {
        ctx.address_mode = AddressMode_Block;
        goto inc_sclk_speed; // SD v2 (block address)
    } else {
        goto set_blocklen; // SD v2 (byte address)
    }

    // TODO: add timeout
app_init_v1:
    const u8 acmdlead_res_v1 =
        sd_send_cmd_r1(SdCmd_AcmdLeading, ACMD_LEADING_ARG, ACMD_LEADING_CRC);

    if (acmdlead_res_v1 == R1_NONE)
        return SdInitResult_ErrNoAcmdLeading;

    const u8 appinit_v1_res =
        sd_send_cmd_r1(SdCmd_AppInit, APP_INIT_ARG_V1, APP_INIT_CRC_V1);

    if (appinit_v1_res == R1_NONE)
        goto init_mmc;
    else if (appinit_v1_res == 0x00)
        goto set_blocklen; // SD v1
    else
        goto app_init_v1;

    // TODO: add timeout
init_mmc:
    const u8 init_res = sd_send_cmd_r1(SdCmd_Init, INIT_ARG, INIT_CRC);

    if (init_res == 0x00) // MMC v3
        goto set_blocklen;
    else if (init_res != R1_NONE)
        goto init_mmc;
    else
        return SdInitResult_ErrNoInit;

set_blocklen:
    ctx.address_mode = AddressMode_Byte;
    const u8 setblocklen_res =
        sd_send_cmd_r1(SdCmd_SetBlockLen, SET_BLOCKLEN_ARG, SET_BLOCKLEN_CRC);

    if (setblocklen_res == 0x00)
        goto inc_sclk_speed;
    else
        return SdInitResult_ErrNoSetBlockLen;

inc_sclk_speed:
    spi_set_hperiod(FREQ_TO_HALF_PERIOD(SPI_FREQ));

    PLIC->int_enable[MeiId_SdDmac] = true;
    ctx.ready = true;

    return SdInitResult_Ok;
}

// NOLINTNEXTLINE
SdReadResult sd_read_block(u32 block_addr, void *const dest)
{
    if (!ctx.ready)
        return SdReadResult_NotReady;

    if (ctx.address_mode == AddressMode_Byte)
        block_addr *= SD_BLOCK_SIZE;

    constexpr u32 READ_SINGLE_BLOCK_CRC = 0x00;
    constexpr u8 START_TOKEN = 0xFE;
    constexpr size_t MAX_READ_ATTEMPTS = 3000;

    SdReadResult result = SdReadResult_Ok;

    spi_write(0xFF);
    spi_cs_enable();
    spi_write(0xFF);

    spi_write(SdCmd_ReadSingleBlock | CMD_START_BITS);
    spi_write_u32(block_addr);
    spi_write(READ_SINGLE_BLOCK_CRC);

    const u8 res = sd_read_r1();

    if (res != 0x00) {
        result = SdReadResult_InvalidRes;
        goto cleanup;
    }

    u8 read = 0xFF;

    for (size_t i = 0; i < MAX_READ_ATTEMPTS; ++i) {
        read = spi_read();
        if (read != 0xFF)
            break;
    }

    if (read != START_TOKEN) {
        result = SdReadResult_TimedOut;
        goto cleanup;
    }

    // u8 *dest_b = dest;
    //
    // for (size_t i = 0; i < SD_BLOCK_SIZE; ++i)
    //     *dest_b++ = spi_read();

    // Start DMA transfer from SPI controller to RAM
    while (!SD_DMAC->ready) {
    }

    ctx.dma_finished = false;
    SD_DMAC->load_dest = dest;
    SD_DMAC->start = 1;

    while (!ctx.dma_finished) {
    }

    // read crc
    const u8 crc_hi = spi_read();
    const u8 crc_lo = spi_read();
    [[maybe_unused]] const u16 crc = concat_u16(crc_hi, crc_lo);

cleanup:
    spi_write(0xFF);
    spi_cs_disable();
    spi_write(0xFF);

    return result;
}

void sd_process_dmac_int()
{
    ctx.dma_finished = true;
}

const char *sd_init_result_str(const SdInitResult init_result)
{
    switch (init_result) {
    case SdInitResult_Ok:
        return "ok";
    case SdInitResult_ErrGoIdleTimedOut:
        return "GO_IDLE timed out or did not respond correctly";
    case SdInitResult_ErrNoInit:
        return "INIT did not respond correctly";
    case SdInitResult_ErrNoSetBlockLen:
        return "SET_BLOCKLEN did not respond correctly";
    default:
        return "unknown init result";
    }
}
