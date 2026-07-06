#ifndef FIRMWARE_SD_CARD_H
#define FIRMWARE_SD_CARD_H

#include "numeric.h"
#include <stddef.h>

constexpr size_t SD_BLOCK_SIZE = 512;

typedef enum {
    SdInitResult_Ok,
    SdInitResult_ErrGoIdleTimedOut,
    SdInitResult_ErrNoInit,
    SdInitResult_ErrNoSetBlockLen,
    SdInitResult_ErrNoAcmdLeading,
} SdInitResult;

typedef enum {
    SdReadResult_Ok,
    SdReadResult_NotReady,
    SdReadResult_InvalidRes,
    SdReadResult_TimedOut,
} SdReadResult;

[[nodiscard]] SdInitResult sd_init();

[[nodiscard]] SdReadResult sd_read_block(u32 block_addr, void *dest);

void sd_process_dmac_int();

const char *sd_init_result_str(SdInitResult init_result);

#endif
