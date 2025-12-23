#pragma once

#include "arena.hpp"
#include "ast.hpp"
#include "stack.hpp"

#include <cstdarg>

namespace ast
{

constexpr size_t PARTIAL_IFS_MAX = 256;

struct partial_ifs {
    ast::_stmt_if_pair *pairs[PARTIAL_IFS_MAX];
    size_t              count = 0;
    bool                has_else;
};

constexpr size_t PARTIAL_BLOCK_MAX = 4096;

struct partial_block {
    ast::stmt *statements[PARTIAL_BLOCK_MAX];
    size_t     count = 0;
};

struct builder {

    arena<stmt> statements  = (1000000);
    arena<exp>  expressions = (1000000);

    arena<void *> ptr_lists = (1000000);

    stack<partial_ifs>   ifs_stack   = (128);
    stack<partial_block> block_stack = (128);

    builder()
    {
    }

    /********************
     *  Internal Stack  *
     ********************/

    void start_block()
    {
        partial_block *partial = block_stack.move_next();

        partial->count = 0;
    }

    stmt *end_block()
    {
        partial_block *partial = block_stack.pop();
        assert(partial->count > 0);

        auto s              = statements.alloc();
        s->kind             = ast::kind::stmt_block;
        s->block.statements = reinterpret_cast<ast::stmt **>(ptr_lists.alloc(partial->count));

        std::memcpy(s->block.statements, partial->statements, partial->count);

        return s;
    }

    void push_block_stmt(stmt *statement)
    {
        partial_block *partial = block_stack.peek();

        assert(partial->count < PARTIAL_BLOCK_MAX);

        partial->statements[partial->count] = statement;
        partial->count++;
    }

    void start_ifs()
    {
        partial_ifs *partial = ifs_stack.move_next();

        partial->count    = 0;
        partial->has_else = false;
    }

    stmt *end_ifs()
    {
        partial_ifs *partial = ifs_stack.pop();
        assert(partial->count > 0);

        auto s       = statements.alloc();
        s->kind      = ast::kind::stmt_ifs;
        s->ifs.pairs = reinterpret_cast<ast::_stmt_if_pair **>(ptr_lists.alloc(partial->count));

        std::memcpy(s->ifs.pairs, partial->pairs, partial->count);

        return s;
    }

    void push_if(exp *condition, stmt *statement)
    {
        partial_ifs *partial = ifs_stack.peek();

        assert(condition != NULL || !partial->has_else);
        assert(partial->count < PARTIAL_IFS_MAX);

        auto s                      = statements.alloc();
        s->kind                     = ast::kind::_stmt_raw_if;
        s->_if_pair.maybe_condition = condition;
        s->_if_pair.statement       = statement;

        partial->pairs[partial->count] = &s->_if_pair;
        partial->has_else              = condition == NULL;
        partial->count++;
    }

    void push_else(stmt *statement)
    {
        return push_if(NULL, statement);
    }

    /****************
     *  Statements  *
     ****************/

    stmt *DBG_stmt(stmt *statement)
    {
        auto s           = statements.alloc();
        s->DBG.statement = statement;
        return s;
    }

    stmt *return_stmt(ast::exp *exp)
    {
        auto s = statements.alloc();

        s->kind                         = ast::kind::stmt_return;
        s->return_stmt.maybe_expression = exp;

        return s;
    }

    /*****************
     *  Expressions  *
     *****************/

    exp *DBG_exp(exp *expression)
    {
        auto e            = expressions.alloc();
        e->DBG.expression = expression;
        return e;
    }

    exp *binary_exp(exp *left, ast::op op, exp *right)
    {
        auto e = expressions.alloc();

        e->kind         = ast::kind::exp_binary;
        e->binary.left  = left;
        e->binary.op    = op;
        e->binary.right = right;

        return e;
    }

    exp *ident(str name)
    {
        auto e = expressions.alloc();

        e->kind       = ast::kind::exp_ident;
        e->ident.name = name;

        return e;
    }

    exp *literal(u64 val)
    {
        auto e = expressions.alloc();

        e->kind            = ast::kind::exp_literal;
        e->literal.kind    = ast::lit_kind::u64;
        e->literal.val_u64 = val;

        return e;
    }
};
}