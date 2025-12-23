#include "interp.hpp"

#include "log.hpp"

namespace backend
{

void interp::exec(bb *block)
{
    block_context ctx = {.block = block, .return_value = NULL};
    while (ctx.block) {
        _exec(ctx);
    }
}

void interp::_exec(block_context &ctx)
{
    bb *block = ctx.block;
    ctx.block = NULL;

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
        case ir::opcode::dbg: {
            u64 val;
            ssa_values.try_get(ssa - 1, &val);

            bool isConst = ((ssa - 1)->flags & ir::ssa_prop::const_val) ==
                           ir::ssa_prop::const_val;

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
        case ir::opcode::nop:
            break;
        case ir::opcode::phi: {
            bool found_phi = false;
            u64  final;
            for (size_t i = 0; i < ssa->right.count; i++) {
                ir::ssa *x = ssa->left.ssa_list[i];
                assert(x != NULL);

                u64 val;
                if (ssa_values.try_get(x, &val)) {
                    assert(!found_phi);
                    final     = val;
                    found_phi = true;
                }
            }
            assert(found_phi);
            ssa_values.add(ssa, final);
            break;
        }
        case ir::opcode::ret:
            ctx.return_value = ssa->left.ssa;
            // TODO JOSH: Block?
            return;
        case ir::opcode::sub: {
            u64 l, r;
            ssa_values.try_get(ssa->left.ssa, &l);
            ssa_values.try_get(ssa->right.ssa, &r);
            ssa_values.add(ssa, l - r);
            break;
        }
        default: {
            log::warnf("not implemented: %.4s",
                       reinterpret_cast<const char *>(&ssa->op));
            break;
        }
        }
    }

    if (block->br == block_br::UNUSED) {
        log::fatalf("Invalid block branch");
        return;
    }
    if (block->br == block_br::unconditional) {
        if (block->block_true) {
            assert(ctx.block == NULL);
            ctx.block = block->block_true;
        }
        return;
    }

    u64 last_ssa;
    assert(ssa_values.try_get(block->tail, &last_ssa));

    bool br_result;
    switch (block->br) {
    case block_br::eq:
        br_result = last_ssa == 0;
        break;
    case block_br::neq:
        br_result = last_ssa != 0;
        break;
    case block_br::lt:
        br_result = last_ssa < 0;
        break;
    case block_br::lte:
        br_result = last_ssa <= 0;
        break;
    case block_br::gt:
        br_result = last_ssa > 0;
        break;
    case block_br::gte:
        br_result = last_ssa >= 0;
        break;
    case block_br::unconditional:
    case block_br::UNUSED:
        log::fatalf("should have already returned");
        return;
    }

    ctx.block = br_result ? block->block_true : block->block_false;
}

}