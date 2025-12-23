
#pragma once

#include "log.hpp"

#include <cstring>
#include <sys/mman.h>

template <typename T>
struct arena {
    T     *head;
    T     *tail;
    size_t cap;
    size_t size;

    arena(size_t max_capacity)
    {
        cap  = max_capacity;
        size = 0;

        void *ptr = mmap(NULL,
                         max_capacity * sizeof(T),
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS,
                         -1,
                         0);

        head = tail = static_cast<T *>(ptr);
    }

    ~arena()
    {
        head = tail = nullptr;
        cap = size = 0;
        munmap(head, cap * sizeof(T));
    }

    T *alloc()
    {
        return alloc(1);
    }

    T *alloc(size_t count)
    {
        size += count;
        if (size > cap) {
            size -= count;
            log::fatalf("Arena reached max capacity (%li)", cap);
        }

        T *ptr = tail;
        tail += count;

        return ptr;
    }
};