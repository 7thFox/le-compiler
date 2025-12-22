
#include "../src/arena.hpp"
#include "../src/basic-block.hpp"
#include "../src/interp.hpp"
#include "../src/ir.hpp"
#include "../src/log.hpp"
#include "../src/map.hpp"

#include <cstdio>

int main()
{

    log::set_severity(log::severity::trace);

    arena<bb>      bb_arena  = (128);
    arena<ir::ssa> ssa_arena = (128);

    auto block0 = bb_arena.alloc();
    auto block1 = bb_arena.alloc();
    auto block2 = bb_arena.alloc();

    // 0
    block0->head       = ssa_arena.tail;
    *ssa_arena.alloc() = {
        .op    = ir::opcode::i,
        .left  = {.i = 4},
        .flags = ir::ssa_prop::const_val,
    };
    // 1
    *ssa_arena.alloc() = {
        .op = ir::opcode::dbg,
    };
    // 2
    *ssa_arena.alloc() = {
        .op    = ir::opcode::i,
        .left  = {.i = 6},
        .flags = ir::ssa_prop::const_val,
    };
    *ssa_arena.alloc() = {
        .op = ir::opcode::dbg,
    };
    *ssa_arena.alloc() = {
        .op    = ir::opcode::add,
        .left  = {.ssa = ssa_arena.tail - 4},
        .right = {.ssa = ssa_arena.tail - 2},
    };
    *ssa_arena.alloc() = {
        .op = ir::opcode::dbg,
    };
    *ssa_arena.alloc() = {
        .op    = ir::opcode::i,
        .left  = {.i = 10},
        .flags = ir::ssa_prop::const_val,
    };
    *ssa_arena.alloc() = {
        .op          = ir::opcode::breq,
        .left        = {.ssa = ssa_arena.tail - 3},
        .right       = {.ssa = ssa_arena.tail - 1},
        .true_block  = block1,
        .false_block = block2,
    };
    block0->tail = ssa_arena.tail - 1;

    block1->head       = ssa_arena.tail;
    *ssa_arena.alloc() = {
        .op    = ir::opcode::i,
        .left  = {.i = 101010},
        .flags = ir::ssa_prop::const_val,
    };
    *ssa_arena.alloc() = {
        .op = ir::opcode::dbg,
    };
    block1->tail = ssa_arena.tail - 1;

    block2->head       = ssa_arena.tail;
    *ssa_arena.alloc() = {
        .op    = ir::opcode::i,
        .left  = {.i = 99999},
        .flags = ir::ssa_prop::const_val,
    };
    *ssa_arena.alloc() = {
        .op = ir::opcode::dbg,
    };
    block2->tail = ssa_arena.tail - 1;

    backend::interp interp = {};
    interp.exec(block0);
}