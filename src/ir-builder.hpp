#pragma once

#include "basic-block.hpp"
#include "ir.hpp"

namespace ir
{

constexpr size_t PARTIAL_PHI_MAX = 256;

struct builder {

    arena<bb>        blocks    = (1000000);
    arena<ir::ssa>   ssas      = (1000000);
    arena<ir::ssa *> phi_lists = (1000000);

    bb *current_block = NULL;

    ir::ssa *current_phi[PARTIAL_PHI_MAX];
    size_t   phi_count = 0;

    builder()
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
        current_block->br   = block_br::UNUSED;
        current_block->head = ssas.tail;
    }

    bb *end_block()
    {
        return end_block(NULL);
    }

    bb *end_block(bb *next)
    {
        assert(current_block->head < ssas.tail /* empty block */);

        bb *block   = current_block;
        block->tail = ssas.tail - 1;
        if (next) {
            assert(current_block->br == block_br::UNUSED);
            assert(block->block_true == NULL);
            assert(block->block_false == NULL);

            block->br         = block_br::unconditional;
            block->block_true = next;
        } else {
            assert(current_block->br != block_br::UNUSED);
        }

        current_block = NULL;

        return block;
    }

    void start_phi()
    {
        assert(phi_count == 0);
    }

    void push_phi(ir::ssa *ssa)
    {
        assert(phi_count < PARTIAL_PHI_MAX);
        current_phi[phi_count] = ssa;
        phi_count++;
    }

    ir::ssa *end_phi()
    {
        assert(phi_count > 0);

        ir::ssa **list = phi_lists.alloc(phi_count);
        std::memcpy(list, current_phi, phi_count * sizeof(ir::ssa *));

        auto ssa           = ssas.alloc();
        ssa->op            = ir::opcode::phi;
        ssa->left.ssa_list = list;
        ssa->right.count   = phi_count;

        phi_count = 0;

        return ssa;
    }

    ir::ssa *DBG()
    {
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

        ssa->op = ir::opcode::dbg;

        return ssa;
    }

    ir::ssa *add(ir::ssa *l, ir::ssa *r)
    {
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

        ssa->op    = ir::opcode::add;
        ssa->left  = {.ssa = l};
        ssa->right = {.ssa = r};

        return ssa;
    }

    void branch(block_br br, bb **when_true, bb **when_false)
    {
        assert(current_block->br == block_br::UNUSED);

        if (*when_true == NULL)
            *when_true = alloc_block();
        if (*when_false == NULL)
            *when_false = alloc_block();

        current_block->br          = br;
        current_block->block_true  = *when_true;
        current_block->block_false = *when_false;
    }

    ir::ssa *literal(u64 val)
    {
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

        ssa->op    = ir::opcode::i;
        ssa->left  = {.i = val};
        ssa->flags = ir::ssa_prop::const_val;

        return ssa;
    }

    ir::ssa *nop()
    {
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

        ssa->op = ir::opcode::nop;

        return ssa;
    }

    ir::ssa *ret()
    {
        return ret(NULL);
    }

    ir::ssa *ret(ir::ssa *val)
    {
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

        ssa->op       = ir::opcode::ret;
        ssa->left.ssa = val;

        current_block->br = block_br::unconditional;

        return ssa;
    }

    ir::ssa *sub(ir::ssa *l, ir::ssa *r)
    {
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

        ssa->op    = ir::opcode::sub;
        ssa->left  = {.ssa = l};
        ssa->right = {.ssa = r};

        return ssa;
    }
};
}