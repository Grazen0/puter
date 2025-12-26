#ifndef FIRMWARE_SD_CARD_H
#define FIRMWARE_SD_CARD_H

#include "numeric.h"
#include <stddef.h>

typedef enum {
    SD_INIT_OK,
    SD_INIT_ERR_NOIDLE,
    SD_INIT_ERR_NOOCR,
    SD_INIT_ERR_NOAPPINIT,
    SD_INIT_ERR_NOINIT,
    SD_INIT_ERR_NOSETBLOCKLEN,
} SdInitResult;

SdInitResult sd_init(void);

void sd_read_block(u32 addr);

const char *sd_init_result_str(SdInitResult init_result);

#endif
