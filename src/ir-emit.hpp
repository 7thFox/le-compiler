#pragma once

#include "arena.hpp"
#include "ast.hpp"
#include "basic-block.hpp"
#include "global.hpp"
#include "ir-builder.hpp"
#include "ir.hpp"
#include "log.hpp"
#include "queue.hpp"

struct comp_unit {
};
struct emit_task;

struct ir_emit {
    ir::builder      *b;
    ast::stmt        *root;
    stack<emit_task> *tasks;
    symbol_table     *local_symbols;
    comp_unit        *compilation_unit;

    ir_emit(ast::stmt        *root,
            ir::builder      *builder,
            stack<emit_task> *tasks,
            symbol_table     *local_symbols,
            comp_unit        *compilation_unit);

    /// @return true if suspended, false if ran to completion
    bool  run_to_suspend();
    SYMID resolve_or_queue(ast::exp_ident *ident);
    void  push_task(emit_task t);

    void emit_stmt(emit_task t);
    void emit_stmt_assign(emit_task t);
    void emit_stmt_block(emit_task t);
    void emit_stmt_ifs(emit_task t);
    void emit_stmt_local_decl(emit_task t);
    void emit_stmt_return(emit_task t);

    void emit_exp(emit_task t);
    void emit_exp_binary(emit_task t);
    void emit_exp_ident(emit_task t);
    void emit_exp_literal(emit_task t);
    void emit_exp_unary(emit_task t);
};

enum class task_type {
    NOT_SET,
    SUSPEND,
    STATEMENT,
    EXPRESSION,
};

enum class exp_mode {
    NOT_SET,
    READ,
    WRITE,
};

struct emit_task {
    task_type type = task_type::NOT_SET;
    size_t    step = 0;
    exp_mode  mode = exp_mode::NOT_SET;

    void (ir_emit::*func)(emit_task);

    union {
        ast::exp  *exp;
        ast::stmt *stmt;
    };

    union {
        ir::ssa *ssa_before;
        ir::ssa *ssa_write;
        size_t   index;
        bb      *when_false;
    } arg1;

    union {
        bb *merge_block;
    } arg2;
};