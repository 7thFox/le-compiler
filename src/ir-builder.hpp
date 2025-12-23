#include "basic-block.hpp"
#include "ir.hpp"

namespace ir
{
struct builder {

    arena<bb>      blocks;
    arena<ir::ssa> ssas;

    bb  *current_block = NULL;
    bool has_terminal  = false;

    builder() : blocks(1000000), ssas(1000000)
    {
    }

    bb *alloc_block()
    {
        return blocks.alloc();
    }

    void start_block()
    {
        start_block(alloc_block());
    }

    void start_block(bb *block)
    {
        assert(block != NULL);
        assert(current_block == NULL);

        current_block       = block;
        has_terminal        = false;
        current_block->head = ssas.tail;
    }

    bb *end_block()
    {
        assert(current_block->head < ssas.tail /* empty block */);
        assert(has_terminal);

        bb *block           = current_block;
        current_block->tail = ssas.tail - 1;
        current_block       = NULL;

        return block;
    }

    ir::ssa *DBG()
    {
        assert(!has_terminal);
        auto ssa = ssas.alloc();

        ssa->op = ir::opcode::dbg;

        return ssa;
    }

    ir::ssa *add(ir::ssa *l, ir::ssa *r)
    {
        assert(!has_terminal);
        auto ssa = ssas.alloc();

        ssa->op    = ir::opcode::add;
        ssa->left  = {.ssa = l};
        ssa->right = {.ssa = r};

        return ssa;
    }

    ir::ssa *branch_eq(ir::ssa *l, ir::ssa *r, bb **when_true, bb **when_false)
    {
        assert(!has_terminal);
        auto ssa = ssas.alloc();

        if (*when_true == NULL)
            *when_true = alloc_block();
        if (*when_false == NULL)
            *when_false = alloc_block();

        ssa->op    = ir::opcode::breq;
        ssa->left  = {.ssa = l};
        ssa->right = {.ssa = r};

        ssa->true_block  = *when_true; // TODO JOSH: move to basic block
        ssa->false_block = *when_false;

        has_terminal = true;

        return ssa;
    }

    ir::ssa *literal(u64 val)
    {
        assert(!has_terminal);
        auto ssa = ssas.alloc();

        ssa->op    = ir::opcode::i;
        ssa->left  = {.i = val};
        ssa->flags = ir::ssa_prop::const_val;

        return ssa;
    }

    ir::ssa *ret(ir::ssa *val)
    {
        assert(!has_terminal);

        // TODO JOSH

        has_terminal = true;

        return NULL;
    }
};
}