#pragma once

#include "basic-block.hpp"
#include "ir.hpp"

#include <new>

namespace ir
{

constexpr size_t PARTIAL_PHI_MAX = 256;

struct builder {

    // arena<scope_t>   scopes    = (1000000);
    arena<bb>        blocks    = (1000000);
    arena<ir::ssa>   ssas      = (1000000);
    arena<ir::ssa *> phi_lists = (1000000);

    // scope_t *current_scope       = NULL;
    bb *current_block       = NULL;
    bb *unconditional_block = NULL;

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
        auto block = unconditional_block != NULL ? unconditional_block : alloc_block();

        unconditional_block = NULL;
        start_block(block);
    }

    void start_block(bb *block)
    {
        assert(block != NULL);
        assert(current_block == NULL);
        // assert(current_scope != NULL);
        assert(unconditional_block == NULL);

        current_block       = block;
        current_block->br   = block_br::UNUSED;
        current_block->head = ssas.next;
        // current_block->scope = current_scope;
    }

    bb *end_block()
    {
        return end_block(NULL);
    }

    bb *end_block(bb *next)
    {
        assert(current_block != NULL);
        bool is_not_empty = current_block->head < ssas.next;
        assert(is_not_empty);

        bb *block   = current_block;
        block->next = ssas.next - 1;
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
        assert(current_block != NULL);
        assert(phi_count == 0);
    }

    void push_phi(ir::ssa *ssa)
    {
        assert(current_block != NULL);
        assert(phi_count < PARTIAL_PHI_MAX);
        current_phi[phi_count] = ssa;
        phi_count++;
    }

    ir::ssa *end_phi()
    {
        assert(current_block != NULL);
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

    // void start_scope()
    // {
    //     assert(current_block == NULL);

    //     auto new_scope = scopes.alloc();
    //     new (new_scope) scope_t();
    //     new_scope->parent = current_scope;

    //     current_scope = new_scope;
    // }

    // void end_scope()
    // {
    //     end_scope(false);
    // }

    // void end_scope(bool is_root)
    // {
    //     assert(current_scope != NULL);
    //     assert(current_block == NULL);
    //     assert(is_root || current_scope->parent != NULL);

    //     current_scope = current_scope->parent;
    // }

    ir::ssa *DBG()
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

        ssa->op    = ir::opcode::dbg;
        ssa->flags = ir::ssa_prop::no_value;

        return ssa;
    }

    ir::ssa *add(ir::ssa *l, ir::ssa *r)
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

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
    }

    void unconditional_branch()
    {
        unconditional_branch(NULL);
    }

    void unconditional_branch(bb *next_block)
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);
        assert(unconditional_block == NULL);

        if (next_block == NULL) // implicit next block
        {
            next_block = unconditional_block = alloc_block();
        }

        current_block->br         = block_br::unconditional;
        current_block->block_true = next_block;
    }

    ir::ssa *literal(u64 val)
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

        ssa->op    = ir::opcode::i;
        ssa->left  = {.i = val};
        ssa->flags = ir::ssa_prop::const_val;

        return ssa;
    }

    ir::ssa *nop()
    {
        assert(current_block != NULL);
        assert(current_block->br == block_br::UNUSED);
        auto ssa = ssas.alloc();

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
        auto ssa = ssas.alloc();

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
        auto ssa = ssas.alloc();

        ssa->op    = ir::opcode::sub;
        ssa->left  = {.ssa = l};
        ssa->right = {.ssa = r};

        return ssa;
    }
};
}