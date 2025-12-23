#include "ast2cfg.hpp"

bb *ast2cfg(ir::builder *b, ast::stmt *tree)
{
    assert(b->blocks.size == 0);

    b->start_block();
    b->nop();
    b->ret();
    b->end_block();

    return b->blocks.head;
}