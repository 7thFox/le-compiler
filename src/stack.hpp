#pragma once

#include "arena.hpp"
#include "global.hpp"

#include <cstdlib>

template <typename T>
struct stack {
    T *max;
    T *bottom;
    T *next;

    stack(T *stack_mem, size_t size)
    {
        bottom = stack_mem;
        next   = bottom;
        max    = bottom + size;
    }

    T *move_next()
    {
        assert(next < max);
        next++;
        return peek();
    }

    T *peek()
    {
        assert(!is_empty());
        return next - 1;
    }

    T *pop()
    {
        assert(!is_empty());
        next--;
        return next;
    }

    bool is_empty()
    {
        return next <= bottom;
    }

    void clear()
    {
        next = bottom;
    }

    size_t count()
    {
        return next - bottom;
    }
};

// template <typename T>
// struct arena_stack {
//     arena<arena<T>> arena_stack;

//     arena_stack(size_t max_arenas, size_t max_per_arean)
// };
