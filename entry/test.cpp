
#include "../src/arena.hpp"
#include "../src/ast-builder.hpp"
#include "../src/ast.hpp"
#include "../src/ast2cfg.hpp"
#include "../src/basic-block.hpp"
#include "../src/global.hpp"
#include "../src/interp.hpp"
#include "../src/ir-builder.hpp"
#include "../src/ir-emit.hpp"
#include "../src/ir.hpp"
#include "../src/log.hpp"
#include "../src/map.hpp"

#include <cstdio>
#include <cstdlib>

// source => tokens
// tokens => AST
// AST => Annotated AST
// Annotated AST reduce (skip)
// Annotatated AST => BB <--------------
//     new Phi instruction make a bb_phi_arena
//     take phi_head, phi_tail
//     add TODO to second-pass phi's for goto/labels
// BB reduce (skip)

constexpr size_t IDK = 100000;

int test_ir(ir::builder &b)
{
    bb *block_entry;
    bb *block_true  = NULL;
    bb *block_false = NULL;

    {
        b.start_block();
        auto l   = b.literal(4);
        auto r   = b.literal(6);
        auto sum = b.add(l, r);
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
        b.ret();
        b.end_block();
    }
    backend::interp interp = {};
    interp.exec(block_entry);

    return 0;
}

int test_ast(ast::builder &b, ir::builder &ir_builder)
{
    // clang-format off
    b.start_block();// main()

    b.push_block_stmt(b.local_decl(&b.ident("u64")->ident,
                                   &b.ident("x")->ident,
                                    b.literal(0x0F00)));

    b.start_ifs();
    b.push_if(
        b.binary_exp(
            b.binary_exp(b.literal(4),
                         ast::op::add,
                         b.literal(6)),
            ast::op::equals,
            b.literal(150)),
        b.assign(b.ident("x"),
                 b.literal(0xCAFE)));

    b.push_else(b.assign(b.ident("x"),
                         b.literal(0xBABE)));

    b.push_block_stmt(b.end_ifs());
    // b.push_block_stmt(b.local_decl(&b.ident("u64")->ident,
    //                                &b.ident("y")->ident,
    //                                 b.ident("x")));
    b.push_block_stmt(b.return_stmt());
    // b.push_block_stmt(b.return_stmt(b.ident("x")));
    // clang-format on

    auto ast = b.end_block();

    // auto block_entry = ast2cfg(&ir_builder, ast);
    auto block_entry = ir_builder.alloc_block();
    ir_builder.start_block(block_entry);

    arena<scope_t>          scopes(IDK);
    arena<sym>              symbols(IDK);
    arena<stack<ir::ssa *>> symbol_ssas(IDK);
    arena<ir::ssa *>        symbol_ssa_mem(IDK);
    symbol_table locals(&scopes, &symbols, &symbol_ssas, &symbol_ssa_mem);

    emit_task        task_mem[1024];
    stack<emit_task> task_queue(task_mem, 1024);
    comp_unit        unit;
    ir_emit          emit(ast, &ir_builder, &task_queue, &locals, &unit);

    bool suspend = emit.run_to_suspend();
    assert(!suspend);
    assert(ir_builder.current_block == NULL);

    backend::interp interp = {};
    interp.exec(block_entry);

    return 0;
}

int main()
{
    log::enable_stacktrace();
    log::set_severity(log::severity::trace);

    // TODO JOSH: Arena size based on page/ physical ram rather than # elements

    arena<bb>        blocks(IDK);
    arena<ir::ssa>   ssas(IDK);
    arena<ir::ssa *> phi_lists(IDK);
    ir::builder      ir_builder(&blocks, &ssas, &phi_lists);

    constexpr size_t MAX_BLOCK_DEPTH = 256;
    constexpr size_t MAX_CHAINED_IFS = 256;

    arena<ast::stmt *> *ip_blocks_mem = static_cast<arena<ast::stmt *> *>(
        std::malloc(MAX_BLOCK_DEPTH * sizeof(arena<void *>)));
    for (size_t i = 0; i < MAX_BLOCK_DEPTH; i++) {
        new (ip_blocks_mem + i) arena<ast::stmt *>(IDK);
    }

    arena<ast::_stmt_if_pair *> *ip_ifs_mem = static_cast<arena<ast::_stmt_if_pair *> *>(
        std::malloc(MAX_CHAINED_IFS * sizeof(arena<void *>)));
    for (size_t i = 0; i < MAX_CHAINED_IFS; i++) {
        new (ip_ifs_mem + i) arena<ast::_stmt_if_pair *>(IDK);
    }

    arena<scope_t>          scopes(IDK);
    arena<sym>              symbols(IDK);
    arena<stack<ir::ssa *>> ssa_stacks(IDK);
    arena<ir::ssa *>        ssa_stacks_mem(IDK);
    symbol_table sym_table(&scopes, &symbols, &ssa_stacks, &ssa_stacks_mem);

    // sym_table.new_scope();
    // auto HACK = sym_table.new_symbol(root_scope);

    arena<u8>                 strs(IDK);
    arena<ast::stmt>          statements(IDK);
    arena<ast::exp>           expressions(IDK);
    arena<void *>             ptr_lists(IDK);
    stack<arena<ast::stmt *>> ip_blocks(ip_blocks_mem, MAX_BLOCK_DEPTH);
    stack<arena<ast::_stmt_if_pair *>> ip_ifs(ip_ifs_mem, MAX_CHAINED_IFS);
    ast::builder ast_builder(&strs, &statements, &expressions, &ptr_lists, &ip_blocks, &ip_ifs);

    // test_ir(ir_builder);
    test_ast(ast_builder, ir_builder);
}