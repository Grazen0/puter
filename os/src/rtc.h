#ifndef FIRMWARE_RTC_H
#define FIRMWARE_RTC_H

#include "numeric.h"

void rtc_init();

void rtc_process_int();

void sleep_ms(u32 ms);

#endif
