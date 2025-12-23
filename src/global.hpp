#pragma once

#include <cstddef>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;

#include "arena.hpp"

struct str {
    std::size_t len;
    const u8   *val;

    str(const char *cstr, arena<u8> arena)
    {
        len = std::strlen(cstr);

        u8 *arr = arena.alloc(len);
        std::memcpy(arr, cstr, len);
        val = arr;
    }
};