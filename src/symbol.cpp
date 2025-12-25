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
}

scope_t *symbol_table::new_scope(scope_t *parent)
{
    auto s    = scopes->alloc();
    s->parent = parent;
    return s;
}

SYMID symbol_table::new_symbol(scope_t *scope)
{
    auto s        = symbols->alloc();
    s->scope_decl = scope;
    s->_ssas      = ssa_stacks->alloc();

    new (s->_ssas) stack<ir::ssa *>(ssa_stacks_mem->alloc(128), 128);
    return s;
}

void sym::push_ssa(ir::ssa *ssa, bb *block)
{
    assert(!ssa->has_flag(ir::ssa_prop::no_value));

    // TODO JOSH: re-alloc stack if too big
    *_ssas->move_next() = ssa;
}

// void sym::pop_ssa(ir::ssa *ssa)
// {
//     assert(*_ssas->pop() == ssa);
// }