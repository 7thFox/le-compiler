
#pragma once

#include "log.hpp"

#include <cstring>
#include <sys/mman.h>

template <typename T>
struct arena {
    T     *head;
    T     *next;
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

        head = next = static_cast<T *>(ptr);
    }

    ~arena()
    {
        munmap(head, cap * sizeof(T));
        head = next = nullptr;
        cap = size = 0;
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

        T *ptr = next;
        next += count;

        return ptr;
    }

    T *peek()
    {
        assert(!is_empty());
        return next - 1;
    }

    bool is_empty()
    {
        return next == head;
    }

    void clear()
    {
        madvise(head, cap * sizeof(T), MADV_DONTNEED);
        next = head;
        size = 0;
    }
};