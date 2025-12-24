#pragma once

#include "global.hpp"
#include "symbol.hpp"

namespace ast
{
enum class op {
    UNUSED,
    // binary
    add,
    sub,
    mul,
    div,
    equals,
    // unary
    boolean_not,
};
enum class lit_kind {
    UNUSED,

    u64,
};

enum class kind {
    UNUSED,
    DBG,

    exp_binary,
    exp_ident,
    exp_literal,
    exp_unary,

    stmt_assign,
    stmt_block,
    stmt_ifs,
    stmt_local_decl,
    stmt_return,
    _stmt_raw_if,
};

struct exp;
struct exp_DBG; // SCAFFOLDING
struct exp_binary;
struct exp_ident;
struct exp_literal;
struct exp_unary;

struct exp_DBG { // SCAFFOLDING
    exp *expression;
};

struct exp_binary {
    exp    *left;
    ast::op op;
    exp    *right;
};

struct exp_ident {
    str   name;
    SYMID anno_symbol;
};

struct exp_literal {
    ast::lit_kind kind;

    union {
        u64 val_u64;
    };
};

struct exp_unary {
    ast::op op;
    exp    *expression;
};

struct stmt;
struct stmt_DBG; // SCAFFOLDING
struct stmt_assign;
struct stmt_block;
struct stmt_ifs;
struct stmt_local_decl;
struct stmt_return;
struct _stmt_if_pair;

struct stmt_DBG {
    // SCAFFOLDING
    stmt *statement;
};

struct stmt_assign {
    exp *left;
    exp *right;
};

struct stmt_block {
    size_t count;
    stmt **statements;
};

struct stmt_else {
    stmt *statement;
};

struct stmt_ifs {
    size_t          count;
    _stmt_if_pair **pairs;
};

struct stmt_local_decl {
    exp_ident *type;
    exp_ident *name;
    exp       *maybe_expression;
};

struct stmt_return {
    exp *maybe_expression;
};

struct _stmt_if_pair {
    exp  *maybe_condition;
    stmt *statement;
};

struct exp {
    ast::kind kind;

    union {
        exp_DBG     DBG;
        exp_binary  binary;
        exp_ident   ident;
        exp_literal literal;
        exp_unary   unary;
        // not actually allowed for raw stmt:
    };
};

struct stmt {
    ast::kind kind;

    union {
        stmt_DBG        DBG;
        stmt_assign     assign;
        stmt_block      block;
        stmt_ifs        ifs;
        stmt_return     return_stmt;
        stmt_local_decl local_decl;

        // not actually allowed for raw stmt:
        _stmt_if_pair _if_pair;
    };
};
}