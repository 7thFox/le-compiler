#pragma once

#include "arena.hpp"
#include "basic-block.hpp"
#include "ir.hpp"
#include "map.hpp"

#include <new>

namespace backend
{
struct block_context {
    bb      *block;
    ir::ssa *return_value;
};

struct interp {
    map<ir::ssa *, u64> ssa_values;

    interp(arena<map_node<ir::ssa *, u64>> *ssa_values);

    void exec(bb *block);
    void _exec(block_context &ctx);
};
}