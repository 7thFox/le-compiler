#pragma once

#include "arena.hpp"
#include "basic-block.hpp"
#include "global.hpp"
#include "ir.hpp"
#include "stack.hpp"

struct sym;
struct symbol_table;
struct scope_t;

constexpr SYMID NO_SYMBOL = NULL;

inline sym *get_symbol(SYMID id)
{
    return static_cast<sym *>(id);
}

struct symbol_table {
    arena<scope_t>          *scopes;
    arena<sym>              *symbols;
    arena<stack<ir::ssa *>> *ssa_stacks;
    arena<ir::ssa *>        *ssa_stacks_mem;

    symbol_table(arena<scope_t>          *scopes,
                 arena<sym>              *symbols,
                 arena<stack<ir::ssa *>> *ssa_stacks,
                 arena<ir::ssa *>        *ssa_stacks_mem);

    scope_t *new_scope(scope_t *parent);
    SYMID    new_symbol(scope_t *scope);
};

struct sym {
    symbol_table     *table;
    scope_t          *scope_decl;
    stack<ir::ssa *> *_ssas;
    stack<bb *>      *_blocks;

    void push_ssa(ir::ssa *ssa, bb *block);
    // void pop_ssa(ir::ssa *ssa);
};

struct scope_t {
    scope_t *parent;
};
