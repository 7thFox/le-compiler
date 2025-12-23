#pragma once

#include "global.hpp"

namespace ir
{

enum class ssa_prop : u64 {
    const_val = 1 << 0,
};

inline ssa_prop operator&(ssa_prop lhs, ssa_prop rhs)
{
    return static_cast<ssa_prop>(static_cast<u64>(lhs) & static_cast<u64>(rhs));
}

inline ssa_prop operator|(ssa_prop lhs, ssa_prop rhs)
{
    return static_cast<ssa_prop>(static_cast<u64>(lhs) | static_cast<u64>(rhs));
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