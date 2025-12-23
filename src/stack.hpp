#pragma once

#include "arena.hpp"
#include "global.hpp"

#include <cstdlib>

template <typename T>
struct stack {
    T *max;
    T *bottom;
    T *next;

    stack(size_t size)
    {
        bottom = static_cast<T *>(std::malloc(sizeof(T) * size));
        next   = bottom;
        max    = bottom + size;
    }

    ~stack()
    {
        std::free(bottom);
    }

    T *move_next()
    {
        assert(next < max);
        next++;
        return peek();
    }

    T *peek()
    {
        assert(next > bottom);
        return next - 1;
    }

    T *pop()
    {
        next--;
        assert(next >= bottom);
        return next;
    }
};
