#include "interp.hpp"

#include "log.hpp"

namespace backend
{

void interp::exec(bb *block)
{
    bb *next = block;
    while (next) {
        next = _exec(next);
    }
}

bb *interp::_exec(bb *block)
{
    for (ir::ssa *ssa = block->head; ssa <= block->tail; ssa++) {

        log::tracef("op: %.4s", reinterpret_cast<const char *>(&ssa->op));

        switch (ssa->op) {
        case ir::opcode::add: {
            u64 l, r;
            ssa_values.try_get(ssa->left.ssa, &l);
            ssa_values.try_get(ssa->right.ssa, &r);
            ssa_values.add(ssa, l + r);
            break;
        }
        case ir::opcode::breq: {
            assert(ssa == block->tail);

            u64 l, r;
            ssa_values.try_get(ssa->left.ssa, &l);
            ssa_values.try_get(ssa->right.ssa, &r);
            if (l == r) {
                return static_cast<bb *>(ssa->true_block);
            } else {
                return static_cast<bb *>(ssa->false_block);
            }
            break;
        }
        case ir::opcode::dbg: {
            u64 val;
            ssa_values.try_get(ssa - 1, &val);

            bool isConst = ((ssa - 1)->flags & ir::ssa_prop::const_val) == ir::ssa_prop::const_val;

            log::debugf("SSA DBG\n"
                        "        Value: %li (0x%X)\n"
                        "        Const: %i",
                        val,
                        val,
                        isConst);
            break;
        }
        case ir::opcode::div: {
            u64 l, r;
            ssa_values.try_get(ssa->left.ssa, &l);
            ssa_values.try_get(ssa->right.ssa, &r);
            ssa_values.add(ssa, l / r);
            break;
        }
        case ir::opcode::i: {
            ssa_values.add(ssa, ssa->left.i);
            break;
        }
        case ir::opcode::mul: {
            u64 l, r;
            ssa_values.try_get(ssa->left.ssa, &l);
            ssa_values.try_get(ssa->right.ssa, &r);
            ssa_values.add(ssa, l * r);
            break;
        }
        case ir::opcode::sub: {
            u64 l, r;
            ssa_values.try_get(ssa->left.ssa, &l);
            ssa_values.try_get(ssa->right.ssa, &r);
            ssa_values.add(ssa, l - r);
            break;
        }
        default: {
            log::warnf("not implemented: %.4s", reinterpret_cast<const char *>(&ssa->op));
            break;
        }
        }
    }

    log::errorf("Basic block does not branch / return");
    return NULL;
}

}