#ifndef FIRMWARE_NUMERIC_H
#define FIRMWARE_NUMERIC_H

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

[[nodiscard]] static inline u16 concat_u16(const u8 hi, const u8 lo)
{
    return ((u16)hi << 8) | (u16)lo;
}

#endif
