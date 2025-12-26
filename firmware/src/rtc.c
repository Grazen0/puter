#include "rtc.h"
#include "numeric.h"
#include "puter.h"

static constexpr size_t MTI_FREQ = 1000;

static u32 ticks;

void rtc_init(void)
{
    RTC->mtime = 0;
    RTC->mtimecmp = RTC_FREQ / MTI_FREQ;
}

void rtc_process_interrupt(void)
{
    RTC->mtime = 0;
    ++ticks;
}

void sleep_ms(const u32 ms)
{
    const u32 start = ticks;
    const u32 end = start + ms;

    if (end < start) {
        while (ticks > start) {
        }
    }

    while (ticks < end) {
    }
}
