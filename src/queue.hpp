#pragma once

#include "global.hpp"

#include <cstdlib>

template <typename T>
struct queue {
    T *root;
    T *out_of_range;
    T *start;
    T *next;

    queue(T *stack_mem, size_t size)
    {
        root         = stack_mem;
        out_of_range = root + size;
        start        = NULL;
        next         = root;
    }

    T *move_next()
    {
        bool out_of_space = next == start;
        assert(!out_of_space);

        auto x = next;
        next++;
        if (next >= out_of_range)
            next = root;

        if (start == NULL) {
            start = x;
        }
        return x;
    }

    T *peek()
    {
        assert(!is_empty());
        return start;
    }

    T *dequeue()
    {
        assert(!is_empty());
        auto x = start;
        start++;
        if (start >= out_of_range)
            start = root;
        if (start == next)
            start = NULL;
        return x;
    }

    bool is_empty()
    {
        return start == NULL;
    }

    void clear()
    {
        start = NULL;
        next  = root;
    }

    size_t count()
    {
        return (out_of_range - start) - (next - root);
    }
};