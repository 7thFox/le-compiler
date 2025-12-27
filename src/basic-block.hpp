#pragma once

#include "ir.hpp"
#include "map.hpp"
#include "symbol.hpp"

enum class block_br {
    UNUSED,

    unconditional,
    eq,
    neq,
    lt,
    gt,
    gte,
    lte,
};

struct bb {
    scope_t              *scope;
    stack<bb *>           predecessors;
    map<SYMID, ir::ssa *> final_values;

    ir::ssa *head;
    ir::ssa *next;

    block_br br;
    bb      *block_true;
    bb      *block_false;
};