#ifndef FIRMWARE_SD_CARD_H
#define FIRMWARE_SD_CARD_H

#include "numeric.h"
#include <stddef.h>

constexpr size_t SD_BLOCK_SIZE = 512;

typedef enum {
    SdInitResult_Ok,
    SdInitResult_ErrNoIdle,
    SdInitResult_ErrNoOcr,
    SdInitResult_ErrNoAppInit,
    SdInitResult_ErrNoInit,
    SdInitResult_ErrNoSetBlockLen,
} SdInitResult;

typedef enum {
    SdReadResult_Ok,
    SdReadResult_NotReady,
    SdReadResult_InvalidRes,
    SdReadResult_TimedOut,
} SdReadResult;

SdInitResult sd_init();

SdReadResult sd_read_block(u32 addr, u8 buf[static SD_BLOCK_SIZE]);

const char *sd_init_result_str(SdInitResult init_result);

#endif
