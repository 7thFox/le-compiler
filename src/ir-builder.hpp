#pragma once

#include "basic-block.hpp"
#include "ir.hpp"

#include <new>

namespace ir
{
struct builder {

    arena<bb>                         *blocks;
    arena<ir::ssa>                    *ssas;
    arena<ir::ssa *>                  *phi_lists;
    arena<bb *>                       *block_lists;
    arena<map_node<SYMID, ir::ssa *>> *value_maps;

    bb       *current_block = NULL;
    ir::ssa **phi_start     = NULL;

    builder(arena<bb>                         *blocks,
            arena<ir::ssa>                    *ssas,
            arena<ir::ssa *>                  *phi_lists,
            arena<bb *>                       *block_lists,
            arena<map_node<SYMID, ir::ssa *>> *value_maps)
    {
        this->blocks      = blocks;
        this->ssas        = ssas;
        this->phi_lists   = phi_lists;
        this->block_lists = block_lists;
        this->value_maps  = value_maps;
    }

    bb *alloc_block()
    {
        auto b = blocks->alloc();
        new (&b->predecessors) stack<bb *>(block_lists->alloc(64), 64);
        new (&b->final_values) map<SYMID, ir::ssa *>(value_maps);
        return b;
    }

    void start_block(bb *block, scope_t *scope)
    {
        assert(block != NULL);
        assert(current_block == NULL);

        current_block        = block;
        current_block->br    = block_br::UNUSED;
        current_block->head  = ssas->next;
        current_block->scope = scope;
    }

    bb *end_block()
    {
        return end_block(NULL);
    }

    bb *end_block(bb *next)
    {
        assert(current_block != NULL);
        bool is_not_empty = current_block->head < ssas->next;
        assert(is_not_empty);

        bb *block   = current_block;
        block->next = ssas->peek();
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
        assert(phi_start == NULL);
        phi_start = phi_lists->next;
    }

    void push_phi(ir::ssa *ssa)
    {
        assert(phi_start != NULL);
        *phi_lists->alloc() = ssa;
    }

    ir::ssa *end_phi()
    {
        assert(phi_start != NULL);
        auto size = phi_lists->next - phi_start;
        assert(size > 0);

        auto ssa           = ssas->alloc();
        ssa->block         = current_block;
        ssa->op            = ir::opcode::phi;
        ssa->left.ssa_list = phi_start;
        ssa->right.count   = size;

        phi_start = NULL;

        return ssa;
    }

    ir::ssa *DBG()
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);

        auto ssa   = ssas->alloc();
        ssa->block = current_block;
        ssa->op    = ir::opcode::dbg;
        ssa->flags = ir::ssa_prop::no_value;

        return ssa;
    }

    ir::ssa *add(ir::ssa *l, ir::ssa *r)
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);

        auto ssa   = ssas->alloc();
        ssa->block = current_block;
        ssa->op    = ir::opcode::add;
        ssa->left  = {.ssa = l};
        ssa->right = {.ssa = r};

        return ssa;
    }

    void branch(block_br br, bb **when_true, bb **when_false)
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);

        if (*when_true == NULL)
            *when_true = alloc_block();
        if (*when_false == NULL)
            *when_false = alloc_block();

        current_block->br          = br;
        current_block->block_true  = *when_true;
        current_block->block_false = *when_false;

        *(*when_true)->predecessors.move_next()  = current_block;
        *(*when_false)->predecessors.move_next() = current_block;
    }

    void unconditional_branch(bb *next_block)
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);

        current_block->br         = block_br::unconditional;
        current_block->block_true = next_block;

        *next_block->predecessors.move_next() = current_block;
    }

    ir::ssa *literal(u64 val)
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);

        auto ssa   = ssas->alloc();
        ssa->block = current_block;
        ssa->op    = ir::opcode::i;
        ssa->left  = {.i = val};
        ssa->flags = ir::ssa_prop::const_val;

        return ssa;
    }

    ir::ssa *nop()
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);

        auto ssa   = ssas->alloc();
        ssa->block = current_block;
        ssa->op    = ir::opcode::nop;
        ssa->flags = ir::ssa_prop::no_value;

        return ssa;
    }

    ir::ssa *ret()
    {
        return ret(NULL);
    }

    ir::ssa *ret(ir::ssa *val)
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);

        auto ssa      = ssas->alloc();
        ssa->block    = current_block;
        ssa->op       = ir::opcode::ret;
        ssa->left.ssa = val;
        ssa->flags    = ir::ssa_prop::no_value;

        current_block->br = block_br::unconditional;

        return ssa;
    }

    ir::ssa *sub(ir::ssa *l, ir::ssa *r)
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);

        auto ssa   = ssas->alloc();
        ssa->block = current_block;
        ssa->op    = ir::opcode::sub;
        ssa->left  = {.ssa = l};
        ssa->right = {.ssa = r};

        return ssa;
    }
};
}