#pragma once

#include <cstddef>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;

struct str {
    std::size_t len;
    const u8   *val;
};