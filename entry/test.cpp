
#include "../src/arena.hpp"
#include "../src/interp.hpp"
#include "../src/ir.hpp"
#include "../src/log.hpp"

#include <cstdio>

int main() {

    log::set_severity(log::severity::trace);

    ir::ssa block[3];

    block[0] = {
        .op   = ir::opcode::i,
        .left = {.i = 4},
    };
    block[1] = {
        .op    = ir::opcode::add,
        .left  = {.i = 6},
        .right = {.ssa = block + 0},
    };
    block[2] = {
        .op = ir::opcode::dbg,
    };

    backend::interp interp = {};
    interp.exec(block, sizeof(block) / sizeof(ir::ssa));
}