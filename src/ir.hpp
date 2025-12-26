#pragma once

#include "global.hpp"

struct bb;

namespace ir
{

enum class ssa_prop : u64 {
    const_val = 1 << 0,
    no_value  = 1 << 1,
};

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
    bb      *block; // TODO JOSH: I don't think we need this

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

    bool has_flag(ssa_prop want)
    {
        u64 flags64 = static_cast<u64>(flags);
        u64 want64  = static_cast<u64>(want);

        return (flags64 & want64) == want64;
    }
};

}