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

    // CONSTS
    i = '46I',

    add = 'DDA',
    sub = 'BUS',
    mul = 'LUB',
    div = 'VID',

    // branches
    breq = 'QERB',

    dbg = 'GBD', // SCAFFOLDING
};

struct ssa {
    opcode   op;
    ssa_prop flags;
    void    *true_block;
    void    *false_block;

    union {
        u64  i;
        ssa *ssa;
    } left;

    union {
        u64  i;
        ssa *ssa;
    } right;
};

}