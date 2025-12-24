#pragma once

#include "global.hpp"
#include "symbol.hpp"

namespace ir
{

enum class ssa_prop : u64 {
    const_val = 1 << 0,
    no_value  = 1 << 1,
};

inline bool has(ssa_prop flags, ssa_prop f)
{
    u64 flags64 = static_cast<u64>(flags);
    u64 f64     = static_cast<u64>(f);

    return (flags64 & f64) == f64;
}

enum class opcode : u32 {
    nop = 'PON',
    phi = 'IHP',

    // CONSTS
    i = '46I',

    add = 'DDA',
    sub = 'BUS',
    mul = 'LUB',
    div = 'VID',

    // branches
    ret = 'TER',

    dbg = 'GBD', // SCAFFOLDING
};

struct ssa {
    opcode   op;
    ssa_prop flags;

    union {
        u64       i;
        ir::ssa  *ssa;
        ir::ssa **ssa_list;
    } left;

    union {
        u64      i;
        ir::ssa *ssa;
        size_t   count;
    } right;
};

}