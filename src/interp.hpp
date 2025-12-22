#pragma once

#include "arena.hpp"
#include "basic-block.hpp"
#include "ir.hpp"
#include "map.hpp"

namespace backend
{
struct interp {

    map<ir::ssa *, u64> ssa_values;

    interp() : ssa_values()
    {
    }

    void exec(bb *block);
    bb  *_exec(bb *block);
};
}