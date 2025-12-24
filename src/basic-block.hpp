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

// constexpr size_t MAX_SCOPE_SYMBOL_SSAS = 512;

// struct scope_symbol {
//     size_t   ssa_count = 0;
//     ir::ssa *ssas[MAX_SCOPE_SYMBOL_SSAS];
// };

// struct scope_t {
//     scope_t *parent;

//     arena<scope_symbol>        scope_symbols = (4096);
//     map<SYMID, scope_symbol *> decl_symbols  = (4096);
// };

struct bb {

    // scope_t *scope;
    ir::ssa *head;
    ir::ssa *next;

    block_br br;
    bb      *block_true;
    bb      *block_false;
};