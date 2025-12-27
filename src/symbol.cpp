#include "symbol.hpp"

#include <new>

symbol_table::symbol_table(arena<scope_t> *scopes, arena<sym> *symbols, arena<SYMID> *sym_lists)
{
    this->scopes    = scopes;
    this->symbols   = symbols;
    this->sym_lists = sym_lists;

    current_scope = NULL;
    log::tracef("  ROOT SCOPE");
    new_scope();
}

void symbol_table::new_scope()
{
    auto s    = scopes->alloc();
    s->parent = current_scope;
    new (&s->decl_symbols) stack<SYMID>(sym_lists->alloc(1024), 1024);

    current_scope = s;

    log::tracef("  START SCOPE   %lx", reinterpret_cast<u64>(current_scope) & 0xFFFF);
}

void symbol_table::end_scope()
{
    log::tracef("  END SCOPE     %lx", reinterpret_cast<u64>(current_scope) & 0xFFFF);
    assert(current_scope->parent != NULL);
    current_scope = current_scope->parent;
    log::tracef("  RESTART SCOPE %lx", reinterpret_cast<u64>(current_scope) & 0xFFFF);
}

SYMID symbol_table::new_symbol(str name)
{
    // TODO JOSH: conflicts
    auto s        = symbols->alloc();
    s->scope_decl = current_scope;

    *current_scope->decl_symbols.move_next() = s;
    return s;
}

SYMID symbol_table::resolve(str name)
{
    // HACK!
    if (name.equal("x")) {
        return symbols->head;
    }
    if (name.equal("y")) {
        return symbols->head + 1;
    }

    return NOT_RESOLVED;
}