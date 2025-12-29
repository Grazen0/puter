#ifndef FIRMWARE_RTC_H
#define FIRMWARE_RTC_H

#include "numeric.h"

void rtc_init();

void rtc_process_interrupt();

void sleep_ms(const u32 ms);

#endif
