#ifndef FIRMWARE_NUMERIC_H
#define FIRMWARE_NUMERIC_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

[[nodiscard]] size_t mod_naive(size_t a, size_t b);

[[nodiscard]] size_t mul(size_t a, size_t b);

[[nodiscard]] static inline u16 concat_u16(const u8 hi, const u8 lo)
{
    return ((u16)hi << 8) | (u16)lo;
}

[[nodiscard]] static inline u32 concat_u32(const u16 hi, const u16 lo)
{
    return ((u32)hi << 16) | (u32)lo;
}

#define get_u16_le(data, idx) get_u16_le(idx, data)
#define get_u32_le(data, idx) get_u32_le(idx, data)

static inline u16(get_u16_le)(const size_t idx, const u8 data[static const idx + 2])
{
    const u16 lo = data[idx];
    const u16 hi = data[idx + 1];
    return (hi << 8) | lo;
}

static inline u32(get_u32_le)(const size_t idx, const u8 data[static const idx + 4])
{
    const u32 a = data[idx];
    const u32 b = data[idx + 1];
    const u32 c = data[idx + 2];
    const u32 d = data[idx + 3];

    return (d << 24) | (c << 16) | (b << 8) | a;
}

#endif
