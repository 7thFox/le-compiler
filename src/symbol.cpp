#include "symbol.hpp"

#include <new>

symbol_table::symbol_table(arena<scope_t>          *scopes,
                           arena<sym>              *symbols,
                           arena<stack<ir::ssa *>> *ssa_stacks,
                           arena<ir::ssa *>        *ssa_stacks_mem)
{
    this->scopes         = scopes;
    this->symbols        = symbols;
    this->ssa_stacks     = ssa_stacks;
    this->ssa_stacks_mem = ssa_stacks_mem;

    current_scope = NULL;
    new_scope();
}

void symbol_table::new_scope()
{
    auto s    = scopes->alloc();
    s->parent = current_scope;

    current_scope = s;
}

void symbol_table::end_scope()
{
    assert(current_scope->parent != NULL);
    current_scope = current_scope->parent;
}

SYMID symbol_table::new_symbol(str name)
{
    auto s        = symbols->alloc();
    s->scope_decl = current_scope;
    s->_ssas      = ssa_stacks->alloc();

    new (s->_ssas) stack<ir::ssa *>(ssa_stacks_mem->alloc(128), 128);
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

void sym::push_ssa(ir::ssa *ssa)
{
    *_ssas->move_next() = ssa;
}