#ifndef FIRMWARE_RTC_H
#define FIRMWARE_RTC_H

#include "numeric.h"

void rtc_init(void);

void rtc_process_interrupt(void);

void sleep_ms(const u32 ms);

#endif
