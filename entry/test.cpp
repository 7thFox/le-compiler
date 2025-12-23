
#include "../src/arena.hpp"
#include "../src/ast-builder.hpp"
#include "../src/ast.hpp"
#include "../src/ast2cfg.hpp"
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

int test_ir()
{
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
        b.sub(sum, b.literal(10));
        b.branch(block_br::eq, &block_true, &block_false);
        block_entry = b.end_block();
    }
    auto b_merge = b.alloc_block();

    b.start_block(block_true);
    auto phi1 = b.literal(0xcafe);
    b.end_block(b_merge);

    b.start_block(block_false);
    auto phi2 = b.literal(0xbabe);
    b.end_block(b_merge);

    {
        b.start_block(b_merge);
        b.start_phi();
        b.push_phi(phi1);
        b.push_phi(phi2);
        b.end_phi();
        b.DBG();
        b.ret();
        b.end_block();
    }
    backend::interp interp = {};
    interp.exec(block_entry);

    return 0;
}

int test_ast()
{
    arena<u8>    strs = (1000000);
    ast::builder b;

    // clang-format off
    b.start_block();// main()

    b.push_block_stmt(b.local_decl(
        &b.ident(str("u64", strs))->ident,
        &b.ident(str("x", strs))->ident));

    b.start_ifs();
    b.push_if(
        b.binary_exp(
            b.binary_exp(
                b.literal(4),
                ast::op::add,
                b.literal(6)),
            ast::op::equals,
            b.literal(10)),
        b.assign(
            b.ident(str("x", strs)),
            b.literal(0xCAFE)));

    b.push_else(
        b.assign(
            b.ident(str("x", strs)),
            b.literal(0xBABE)));
    // clang-format on

    auto ast = b.end_block();

    ir::builder ir_builder;
    auto        block_entry = ast2cfg(&ir_builder, ast);

    backend::interp interp = {};
    interp.exec(block_entry);

    return 0;
}

int main()
{
    log::enable_stacktrace();
    log::set_severity(log::severity::trace);

    // test_ir();
    test_ast();
}