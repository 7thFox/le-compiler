#include "interp.hpp"

#include "log.hpp"

namespace backend
{

interp::interp(arena<map_node<ir::ssa *, u64>> *values_arena)
    : ssa_values(values_arena)
{
}

void interp::exec(bb *block)
{
    assert(block != NULL);

    block_context ctx = {.block = block, .return_value = NULL};
    while (ctx.block != NULL) {
        _exec(ctx);
    }
}

void interp::_exec(block_context &ctx)
{
    assert(ctx.block != NULL);
    assert(ctx.block->head != NULL);

    bb *block = ctx.block;
    ctx.block = NULL;

    for (ir::ssa *ssa = block->head; ssa <= block->next; ssa++) {

        char param_buff[256];
        bool do_break          = false;
        bool print_2_addresses = false;
        param_buff[0]          = 0;

        switch (ssa->op) {
        case ir::opcode::add: {
            print_2_addresses = true;
            u64 l             = -1;
            u64 r             = -1;

            ssa_values.try_get(ssa->left.ssa, &l);
            ssa_values.try_get(ssa->right.ssa, &r);
            ssa_values.add(ssa, l + r);
            break;
        }
        case ir::opcode::div: {
            print_2_addresses = true;
            u64 l             = -1;
            u64 r             = -1;

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
            print_2_addresses = true;
            u64 l             = -1;
            u64 r             = -1;

            ssa_values.try_get(ssa->left.ssa, &l);
            ssa_values.try_get(ssa->right.ssa, &r);
            ssa_values.add(ssa, l * r);
            break;
        }
        case ir::opcode::nop:
            break;
        case ir::opcode::phi: {
            ir::ssa *found_phi = NULL;
            u64      final     = -1;
            char    *buff      = param_buff;

            for (size_t i = 0; i < ssa->right.count; i++) {
                ir::ssa *x = ssa->left.ssa_list[i];
                assert(x != NULL);

                u64 val;
                // phi is ordered. the first one is the correct one. only continuing to print.
                if (!found_phi && ssa_values.try_get(x, &val)) {
                    final     = val;
                    found_phi = x;

                    buff += snprintf(buff,
                                     sizeof(param_buff) - (buff - param_buff),
                                     "%s$%04lx %s",
                                     log::COLOR_DEBUG,
                                     (u64)x & 0xFFFF,
                                     log::COLOR_TRACE);
                } else {

                    buff += snprintf(buff,
                                     sizeof(param_buff) - (buff - param_buff),
                                     "$%04lx ",
                                     (u64)x & 0xFFFF);
                }
            }
            assert(found_phi);
            ssa_values.add(ssa, final);
            break;
        }
        case ir::opcode::ret:
            ctx.return_value = ssa->left.ssa;
            break;
        case ir::opcode::sub: {
            u64 l = -1;
            u64 r = -1;

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

        if (print_2_addresses) {
            snprintf(param_buff,
                     sizeof(param_buff),
                     "$%04lx $%04lx",
                     (u64)ssa->left.ssa & 0xFFFF,
                     (u64)ssa->right.ssa & 0xFFFF);
        }

        char result_buff[64];
        result_buff[0] = 0;
        u64 ssa_result = -1;
        if (ssa_values.try_get(ssa, &ssa_result)) {
            snprintf(result_buff, sizeof(result_buff), "0x%04lX (%5li)", ssa_result, ssa_result);
        }
        log::tracef("[%06lX] $%04lx: %.4s\t%s\t%s",
                    (u64)block & 0xFFFFFF,
                    (u64)ssa & 0xFFFF,
                    reinterpret_cast<const char *>(&ssa->op),
                    result_buff,
                    param_buff);

        if (do_break) {
            return;
        }
    }

    if (block->br == block_br::UNUSED) {
        log::fatalf("Invalid block branch");
        return;
    }

    if (block->br == block_br::unconditional) {
        if (block->block_true != NULL) {
            assert(ctx.block == NULL);
            ctx.block = block->block_true;
        }
        log::tracef("[%06lX] br  $%06lX", (u64)block & 0xFFFFFF, (u64)ctx.block & 0xFFFFFF);
        return;
    }

    u64 last_ssa = -1;
    assert(ssa_values.try_get(block->next, &last_ssa));

    const char *br_dsp = "???";
    bool        br_result;
    switch (block->br) {
    case block_br::eq:
        br_dsp    = "eq";
        br_result = last_ssa == 0;
        break;
    case block_br::neq:
        br_dsp    = "neq";
        br_result = last_ssa != 0;
        break;
    case block_br::lt:
        br_dsp    = "lt";
        br_result = last_ssa < 0;
        break;
    case block_br::lte:
        br_dsp    = "lte";
        br_result = last_ssa <= 0;
        break;
    case block_br::gt:
        br_dsp    = "gt";
        br_result = last_ssa > 0;
        break;
    case block_br::gte:
        br_dsp    = "gte";
        br_result = last_ssa >= 0;
        break;
    case block_br::unconditional:
    case block_br::UNUSED:
        log::fatalf("should have already returned");
        return;
    }

    ctx.block = br_result ? block->block_true : block->block_false;

    log::tracef("[%06lX] %-3s t:$%06lX f:$%06lX",
                (u64)block & 0xFFFFFF,
                br_dsp,
                (u64)block->block_true & 0xFFFFFF,
                (u64)block->block_false & 0xFFFFFF);
}

}