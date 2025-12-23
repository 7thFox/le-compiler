
#include "../src/arena.hpp"
#include "../src/ast-builder.hpp"
#include "../src/ast.hpp"
#include "../src/basic-block.hpp"
#include "../src/global.hpp"
#include "../src/interp.hpp"
#include "../src/ir-builder.hpp"
#include "../src/ir.hpp"
#include "../src/log.hpp"
#include "../src/map.hpp"

#include <cstdio>

// source => tokens
// tokens => AST
// AST => Annotated AST
// Annotated AST reduce (skip)
// Annotatated AST => BB <--------------
//     new Phi instruction make a bb_phi_arena
//     take phi_head, phi_tail
//     add TODO to second-pass phi's for goto/labels
// BB reduce (skip)

int main2();

int main()
{
    log::enable_stacktrace();
    log::set_severity(log::severity::trace);

    arena<u8>    strs = (1000000);
    ast::builder b;
    // clang-format off

    b.start_ifs();

    b.start_block();
    b.push_block_stmt(b.return_stmt(b.DBG_exp(b.literal(0xCAFE))));
    auto when_true = b.end_block();

    b.start_block();
    b.push_block_stmt(b.return_stmt(b.DBG_exp(b.literal(0xBABE))));
    auto when_false = b.end_block();

    b.push_if(
        b.binary_exp(
            b.binary_exp(
                b.literal(4),
                ast::op::add,
                b.literal(6)),
            ast::op::equals,
            b.literal(10)),
        when_true);
    b.push_else(when_false);

    auto ast = b.end_ifs();
    // clang-format on

    // // clang-format off
    // auto ast = b.binary_exp(
    //     b.ident(str("x", strs)),
    //     ast::op::add,
    //      b.ident(str("y", strs)));
    // // clang-format on
    // b.ident()

    return 0;
}

int main2()
{

    log::set_severity(log::severity::trace);

    ir::builder b;

    bb *block_entry;
    bb *block_true  = NULL;
    bb *block_false = NULL;

    {
        b.start_block();
        auto l = b.literal(4);
        b.DBG();
        auto r = b.literal(6);
        b.DBG();
        auto sum = b.add(l, r);
        b.DBG();
        auto cmp = b.literal(10);
        b.branch_eq(sum, cmp, &block_true, &block_false);
        block_entry = b.end_block();
    }
    {
        b.start_block(block_true);
        b.literal(0xcafe);
        b.DBG();
        b.ret(NULL);
        b.end_block();
    }
    {
        b.start_block(block_false);
        b.literal(0xbabe);
        b.DBG();
        b.ret(NULL);
        b.end_block();
    }
    backend::interp interp = {};
    interp.exec(block_entry);

    return 0;
}