#pragma once

#include "arena.hpp"
#include "global.hpp"
#include "ir.hpp"
#include "stack.hpp"

struct sym;
struct symbol_table;
struct scope_t;

typedef sym *SYMID;

constexpr SYMID NOT_RESOLVED = NULL;

inline sym *get_symbol(SYMID id)
{
    return static_cast<sym *>(id);
}

struct symbol_table {
    arena<scope_t> *scopes;
    arena<sym>     *symbols;
    arena<SYMID>   *sym_lists;

    scope_t *current_scope;

    symbol_table(arena<scope_t> *scopes, arena<sym> *symbols, arena<SYMID> *sym_lists);

    void  new_scope();
    void  end_scope();
    SYMID new_symbol(str name);
    SYMID resolve(str name);
};

struct sym {
    symbol_table *table;
    scope_t      *scope_decl;
};

struct scope_t {
    scope_t     *parent;
    stack<SYMID> decl_symbols;
};
