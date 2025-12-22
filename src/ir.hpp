#pragma once

#include "global.hpp"

namespace ir {

enum class opcode : u32 {

    // CONSTS
    i = '46I',

    add = 'DDA',
    sub = 'BUS',
    mul = 'LUB',
    div = 'VID',

    dbg = 'GBD', // SCAFFOLDING
};

struct ssa {
    opcode op;

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