#pragma once

#include "ir.hpp"
#include "map.hpp"

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
    ir::ssa *head;
    ir::ssa *next;

    block_br br;
    bb      *block_true;
    bb      *block_false;
};