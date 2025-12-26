#pragma once

#include "arena.hpp"
#include "ast.hpp"
#include "stack.hpp"

#include <cstdarg>

namespace ast
{
constexpr size_t MAX_STACKS = 4096 / sizeof(void *);

struct builder {

    arena<u8>                          *strs;
    arena<ast::stmt>                   *statements;
    arena<ast::exp>                    *expressions;
    arena<void *>                      *ptr_lists;
    stack<arena<ast::stmt *>>          *ip_blocks;
    stack<arena<ast::_stmt_if_pair *>> *ip_ifs;

    builder(arena<u8>                          *strs,
            arena<ast::stmt>                   *statements,
            arena<ast::exp>                    *expressions,
            arena<void *>                      *ptr_lists,
            stack<arena<ast::stmt *>>          *ip_blocks,
            stack<arena<ast::_stmt_if_pair *>> *ip_ifs)
    {
        this->strs        = strs;
        this->statements  = statements;
        this->expressions = expressions;
        this->ptr_lists   = ptr_lists;
        this->ip_blocks   = ip_blocks;
        this->ip_ifs      = ip_ifs;
    }

    /********************
     *  Internal Stack  *
     ********************/

    void start_block()
    {
        auto ip = ip_blocks->move_next();
        ip->clear();
    }

    stmt *end_block()
    {
        auto ip = ip_blocks->pop();
        assert(!ip->is_empty());

        auto s         = statements->alloc();
        s->kind        = ast::kind::stmt_block;
        s->block.count = ip->size;
        s->block.statements =
            reinterpret_cast<ast::stmt **>(ptr_lists->alloc(s->block.count));

        std::memcpy(s->block.statements, ip->head, ip->size * sizeof(ast::stmt *));

        return s;
    }

    void push_block_stmt(stmt *statement)
    {
        assert(statement != NULL);

        auto ip      = ip_blocks->peek();
        *ip->alloc() = statement;
    }

    void start_ifs()
    {
        auto ip = ip_ifs->move_next();
        ip->clear();
    }

    stmt *end_ifs()
    {
        auto ip = ip_ifs->pop();
        assert(!ip->is_empty());

        auto s       = statements->alloc();
        s->kind      = ast::kind::stmt_ifs;
        s->ifs.count = ip->size;
        s->ifs.pairs =
            reinterpret_cast<ast::_stmt_if_pair **>(ptr_lists->alloc(s->ifs.count));

        std::memcpy(s->ifs.pairs, ip->head, ip->size * sizeof(ast::_stmt_if_pair *));

        return s;
    }

    void push_if(exp *condition, stmt *statement)
    {
        assert(statement != NULL);

        auto ip = ip_ifs->peek();

        bool else_first = condition == NULL && ip->is_empty();
        assert(!else_first);
        bool multiple_else = condition == NULL && (*ip->peek())->maybe_condition == NULL;
        assert(!multiple_else);

        auto s                      = statements->alloc();
        s->kind                     = ast::kind::_stmt_raw_if;
        s->_if_pair.maybe_condition = condition;
        s->_if_pair.statement       = statement;

        *ip->alloc() = &s->_if_pair;
    }

    void push_else(stmt *statement)
    {
        assert(statement != NULL);

        return push_if(NULL, statement);
    }

    /****************
     *  Statements  *
     ****************/

    stmt *DBG_stmt(stmt *statement)
    {
        assert(statement != NULL);

        auto s           = statements->alloc();
        s->DBG.statement = statement;
        return s;
    }

    stmt *assign(ast::exp *left, ast::exp *right)
    {
        auto s = statements->alloc();

        s->kind         = ast::kind::stmt_assign;
        s->assign.left  = left;
        s->assign.right = right;

        return s;
    }

    stmt *return_stmt()
    {
        return return_stmt(NULL);
    }

    stmt *return_stmt(ast::exp *exp)
    {
        auto s = statements->alloc();

        s->kind                         = ast::kind::stmt_return;
        s->return_stmt.maybe_expression = exp;

        return s;
    }

    stmt *local_decl(exp_ident *type, exp_ident *name)
    {
        return local_decl(type, name, NULL);
    }

    stmt *local_decl(exp_ident *type, exp_ident *name, exp *expression)
    {
        assert(type != NULL);
        assert(name != NULL);

        auto s = statements->alloc();

        s->kind                        = ast::kind::stmt_local_decl;
        s->local_decl.type             = type;
        s->local_decl.name             = name;
        s->local_decl.maybe_expression = expression;

        return s;
    }

    /*****************
     *  Expressions  *
     *****************/

    exp *DBG_exp(exp *expression)
    {
        assert(expression != NULL);

        auto e            = expressions->alloc();
        e->DBG.expression = expression;
        return e;
    }

    exp *binary_exp(exp *left, ast::op op, exp *right)
    {
        assert(left != NULL);
        assert(right != NULL);
        assert(op != ast::op::UNUSED);

        auto e = expressions->alloc();

        e->kind         = ast::kind::exp_binary;
        e->binary.left  = left;
        e->binary.op    = op;
        e->binary.right = right;

        return e;
    }

    exp *ident(const char *name)
    {
        return ident(str(name, strs));
    }

    exp *ident(str name)
    {
        assert(name.len > 0);

        auto e = expressions->alloc();

        e->kind       = ast::kind::exp_ident;
        e->ident.name = name;

        return e;
    }

    exp *literal(u64 val)
    {
        auto e = expressions->alloc();

        e->kind            = ast::kind::exp_literal;
        e->literal.kind    = ast::lit_kind::u64;
        e->literal.val_u64 = val;

        return e;
    }
};
}