#pragma once

#include "arena.hpp"
#include "global.hpp"
#include "log.hpp"

#include <functional>

template <typename TKey, typename TValue>
struct map_node {
    bool is_init;
    bool is_leaf;

    union {
        struct {
            TKey   key;
            TValue value;
        } leaf;

        map_node<TKey, TValue> *buckets;
    };
};

static constexpr size_t N_BUCKETS = 256;

template <typename TKey, typename TValue>
struct map {
    arena<map_node<TKey, TValue>> *bucket_arena;
    map_node<TKey, TValue>        *buckets;

    map(arena<map_node<TKey, TValue>> *bucket_arena)
    {
        this->bucket_arena = bucket_arena;
        buckets            = bucket_arena->alloc(N_BUCKETS);
    }

    void add(TKey key, TValue value)
    {
        auto hash = std::hash<TKey>{}(key);
        bucket_add(key, value, hash, 0, buckets);
    }

    void bucket_add(TKey key, TValue value, size_t hash, int depth, map_node<TKey, TValue> *bucket)
    {
        assert(bucket != NULL);
        assert(depth < 8);

        u8 hpart = (hash >> (8 * depth)) & 0xFF;

        auto node = bucket + hpart;

        if (!node->is_init) {
            node->is_init    = true;
            node->is_leaf    = true;
            node->leaf.key   = key;
            node->leaf.value = value;
            return;
        }

        if (node->is_leaf) {
            if (node->leaf.key == key) {
                log::fatalf("Attempt to add duplicate key");
                return;
            }

            auto new_bucket = bucket_arena->alloc(N_BUCKETS);
            bucket_add(node->leaf.key,
                       node->leaf.value,
                       std::hash<TKey>{}(node->leaf.key),
                       depth + 1,
                       new_bucket);

            node->is_leaf = false;
            node->buckets = new_bucket;
        }

        bucket_add(key, value, hash, depth + 1, node->buckets);
    }

    bool try_get(TKey key, TValue *value)
    {
        auto hash = std::hash<TKey>{}(key);
        return bucket_get(key, value, hash, 0, buckets);
    }

    bool bucket_get(TKey key, TValue *value, size_t hash, int depth, map_node<TKey, TValue> *bucket)
    {
        assert(depth < 8);
        assert(bucket != NULL);

        u8 hpart = (hash >> (8 * depth)) & 0xFF;

        auto node = bucket + hpart;

        if (!node->is_init) {
            return false;
        }

        if (node->is_leaf) {
            if (node->leaf.key == key) {
                *value = node->leaf.value;
                return true;
            }
            return false;
        }

        return bucket_get(key, value, hash, depth + 1, node->buckets);
    }
};
