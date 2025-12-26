#include "ir-emit.hpp"

ir_emit::ir_emit(ast::stmt        *root,
                 ir::builder      *b,
                 stack<emit_task> *tasks,
                 symbol_table     *local_symbols,
                 comp_unit        *compilation_unit)
{
    this->root             = root;
    this->b                = b;
    this->tasks            = tasks;
    this->local_symbols    = local_symbols;
    this->compilation_unit = compilation_unit;

    push_task({
        .type = task_type::STATEMENT,
        .func = &ir_emit::emit_stmt,
        .step = 0,
        .stmt = root,
    });
}

void ir_emit::push_task(emit_task t)
{
    assert(t.type != task_type::NOT_SET);
    assert(t.func != NULL);

    switch (t.type) {
    case task_type::EXPRESSION:
        assert(t.exp != NULL);
        assert(t.mode != exp_mode::NOT_SET);
        if (t.mode == exp_mode::WRITE) {
            assert(t.arg1.ssa_write != NULL);
        }
        break;
    case task_type::STATEMENT:
        assert(t.stmt != NULL);
        break;
    }

    *tasks->move_next() = t;
}

bool ir_emit::run_to_suspend()
{
    u64 task_count = 0;
    while (!tasks->is_empty()) {
        auto task = tasks->pop();
        task_count++;
        if (task->type == task_type::SUSPEND) {
            return true;
        }
        (this->*(task->func))(*task);
    }

    if (b->current_block != NULL) {
        if (b->current_block->br == block_br::UNUSED) {
            log::warnf("no return from last block. implied ret");
            b->ret();
        }
        b->end_block();
    }

    log::debugf("Ended emit after %li tasks", task_count);
    return false;
}

void ir_emit::emit_stmt(emit_task t)
{
    assert(t.step == 0);

    ast::stmt *s = t.stmt;

    switch (s->kind) {
    case ast::kind::stmt_assign:
        emit_stmt_assign(t);
        return;
    case ast::kind::stmt_ifs:
        emit_stmt_ifs(t);
        return;
    case ast::kind::stmt_local_decl:
        emit_stmt_local_decl(t);
        return;
    case ast::kind::stmt_return:
        emit_stmt_return(t);
        return;
    case ast::kind::stmt_block:
        emit_stmt_block(t);
        return;
    }

    log::warnf("Statement kind not handled (%i)", s->kind);
}

void ir_emit::emit_stmt_assign(emit_task t)
{
    log::tracef("emit_stmt_assign (%i)", t.step);

    auto *s = &t.stmt->assign;
    switch (t.step) {
    case 0: {
        push_task({
            .type            = task_type::STATEMENT,
            .func            = &ir_emit::emit_stmt_assign,
            .step            = 1,
            .stmt            = t.stmt,
            .arg1.ssa_before = b->ssas->peek_permissive(),
        });

        ir_emit::emit_exp({
            .type = task_type::EXPRESSION,
            .mode = exp_mode::READ,
            .step = 0,
            .exp  = s->right,
        });
        return;
    }
    case 1: {
        ir::ssa *val = b->ssas->peek();
        assert(val != t.arg1.ssa_before);
        assert(!val->has_flag(ir::ssa_prop::no_value));

        ir_emit::emit_exp({
            .type           = task_type::EXPRESSION,
            .mode           = exp_mode::WRITE,
            .step           = 0,
            .exp            = s->left,
            .arg1.ssa_write = val,
        });
        return;
    }
    }
    log::fatalf("invalid step %li", t.step);
}

void ir_emit::emit_stmt_block(emit_task t)
{
    log::tracef("emit_stmt_block (%i)", t.step);
    ast::stmt_block *s = &t.stmt->block;

    switch (t.step) {
    case 0: {
        if (s->count == 0) {
            b->nop();
            return;
        }

        local_symbols->new_scope();
        *tasks->move_next() = {
            .type = task_type::STATEMENT,
            .func = &ir_emit::emit_stmt_block,
            .step = 1,
            .stmt = t.stmt,
        };

        for (size_t i = 0; i < s->count; i++) {
            // push in reverse order:
            *tasks->move_next() = {
                .type = task_type::STATEMENT,
                .func = &ir_emit::emit_stmt,
                .step = 0,
                .stmt = s->statements[s->count - i - 1],
            };
        }

        return;
    }
    case 1: {
        local_symbols->end_scope();
        return;
    }
    }
    log::fatalf("invalid step %li", t.step);
}

void ir_emit::emit_stmt_ifs(emit_task t)
{
    log::tracef("emit_stmt_ifs [%li] (%i)", t.arg1.index, t.step);

    constexpr int STEP_AFTER_CONDITION = 1;
    constexpr int STEP_AFTER_STATEMENT = 2;
    constexpr int STEP_MERGE           = 3;

    auto *s = &t.stmt->ifs;
    switch (t.step) {
    case 0: {

        assert(b->current_block != NULL);
        assert(s->count > 0);

        auto merge_block = b->alloc_block();

        push_task({
            .type             = task_type::STATEMENT,
            .func             = &ir_emit::emit_stmt_ifs,
            .step             = STEP_MERGE,
            .stmt             = t.stmt,
            .arg2.merge_block = merge_block,
        });

        for (size_t i = 0; i < s->count; i++) {
            auto index = s->count - i - 1;
            auto pair  = s->pairs[index];

            if (pair->maybe_condition != NULL) {
                push_task({
                    .type             = task_type::STATEMENT,
                    .func             = &ir_emit::emit_stmt_ifs,
                    .step             = STEP_AFTER_CONDITION,
                    .stmt             = t.stmt,
                    .arg1.index       = index,
                    .arg2.merge_block = merge_block,
                });
                push_task({
                    .type = task_type::EXPRESSION,
                    .mode = exp_mode::READ,
                    .func = &ir_emit::emit_exp,
                    .step = 0,
                    .exp  = pair->maybe_condition,
                });
            } else {
                push_task({
                    .type             = task_type::STATEMENT,
                    .func             = &ir_emit::emit_stmt_ifs,
                    .step             = STEP_AFTER_CONDITION,
                    .stmt             = t.stmt,
                    .arg1.index       = index,
                    .arg2.merge_block = merge_block,
                });
            }
        }
        return;
    }
    case STEP_AFTER_CONDITION: {
        auto pair = s->pairs[t.arg1.index];
        assert(pair->statement != NULL);
        assert(t.arg2.merge_block != NULL);

        if (pair->maybe_condition == NULL) {
            assert(t.arg1.index == s->count - 1);
            push_task({
                .type             = task_type::STATEMENT,
                .func             = &ir_emit::emit_stmt_ifs,
                .step             = STEP_AFTER_STATEMENT,
                .stmt             = t.stmt,
                .arg2.merge_block = t.arg2.merge_block,
            });

            ir_emit::emit_stmt({
                .type = task_type::STATEMENT,
                .step = 0,
                .stmt = pair->statement,
            });
        } else {

            auto condition = b->ssas->peek();
            assert(!condition->has_flag(ir::ssa_prop::no_value));

            bb *when_true  = NULL;
            bb *when_false = NULL;
            b->branch(block_br::eq, &when_true, &when_false);
            b->end_block();
            b->start_block(when_true);
            push_task({
                .type             = task_type::STATEMENT,
                .func             = &ir_emit::emit_stmt_ifs,
                .step             = STEP_AFTER_STATEMENT,
                .stmt             = t.stmt,
                .arg1.when_false  = when_false,
                .arg2.merge_block = t.arg2.merge_block,
            });
            ir_emit::emit_stmt({
                .type = task_type::STATEMENT,
                .step = 0,
                .stmt = pair->statement,
            });
        }

        return;
    }
    case STEP_AFTER_STATEMENT: {

        assert(t.arg2.merge_block != NULL);

        b->unconditional_branch(t.arg2.merge_block);
        b->end_block();
        if (t.arg1.when_false != NULL) {
            // if statement

            b->start_block(t.arg1.when_false);
        }

        return;
    }
    case STEP_MERGE: {
        assert(t.arg2.merge_block != NULL);
        if (b->current_block != NULL) { // no else
            b->unconditional_branch(t.arg2.merge_block);
            b->end_block();
        }

        b->start_block(t.arg2.merge_block);
        return;
    }
    }
    log::fatalf("invalid step %li", t.step);
}

void ir_emit::emit_stmt_local_decl(emit_task t)
{
    log::tracef("emit_stmt_local_decl (%i)", t.step);

    ast::stmt_local_decl *s = &t.stmt->local_decl;

    switch (t.step) {
    case 0: {
        assert(s->type->name.equal("u64")); // HACK
        local_symbols->new_symbol(s->name->name);

        if (s->maybe_expression != NULL) {
            push_task({
                .type            = task_type::STATEMENT,
                .func            = &ir_emit::emit_stmt_local_decl,
                .step            = 1,
                .stmt            = t.stmt,
                .arg1.ssa_before = b->ssas->peek_permissive(),
            });
            ir_emit::emit_exp({
                .type = task_type::EXPRESSION,
                .mode = exp_mode::READ,
                .step = 0,
                .exp  = s->maybe_expression,
            });
        }
        return;
    }
    case 1: {
        assert(s->maybe_expression != NULL);

        ir::ssa *exp_result = b->ssas->peek();
        assert(exp_result != t.arg1.ssa_before);
        assert(!exp_result->has_flag(ir::ssa_prop::no_value));

        auto sym = local_symbols->resolve(s->name->name);
        assert(sym != NOT_RESOLVED);
        sym->push_ssa(exp_result);
        return;
    }
    }
    log::fatalf("invalid step %li", t.step);
}

void ir_emit::emit_stmt_return(emit_task t)
{
    log::tracef("emit_stmt_return (%i)", t.step);

    ast::stmt_return *s = &t.stmt->return_stmt;
    switch (t.step) {
    case 0: {
        if (s->maybe_expression == NULL) {
            b->ret(NULL);
            b->end_block();
            return;
        }
        push_task({
            .type            = task_type::STATEMENT,
            .func            = &ir_emit::emit_stmt_return,
            .step            = 1,
            .stmt            = t.stmt,
            .arg1.ssa_before = b->ssas->peek_permissive(),
        });

        ir_emit::emit_exp({
            .type = task_type::EXPRESSION,
            .mode = exp_mode::READ,
            .step = 0,
            .exp  = s->maybe_expression,
        });
        return;
    }
    case 1: {

        assert(s->maybe_expression != NULL);
        ir::ssa *val = b->ssas->peek();
        assert(val != t.arg1.ssa_before);
        assert(!val->has_flag(ir::ssa_prop::no_value));

        b->ret(val);
        b->end_block();
        return;
    }
    }
    log::fatalf("invalid step %li", t.step);
}

void ir_emit::emit_exp(emit_task t)
{
    assert(t.step == 0);

    auto e = t.exp;

    switch (e->kind) {
    case ast::kind::exp_binary:
        emit_exp_binary(t);
        return;
    case ast::kind::exp_ident:
        emit_exp_ident(t);
        return;
    case ast::kind::exp_literal:
        emit_exp_literal(t);
        return;
    }

    log::warnf("Expression kind not handled (%i)", e->kind);
}

void ir_emit::emit_exp_binary(emit_task t)
{
    log::tracef("emit_exp_binary (%i)", t.step);
    auto e = &t.exp->binary;

    switch (t.step) {
    case 0: {
        assert(t.mode == exp_mode::READ);
        assert(e->left != NULL);
        assert(e->right != NULL);
        assert(e->op != ast::op::UNUSED);

        push_task({
            .type            = task_type::EXPRESSION,
            .mode            = t.mode,
            .func            = &ir_emit::emit_exp_binary,
            .step            = 1,
            .exp             = t.exp,
            .arg1.ssa_before = b->ssas->peek_permissive(),
        });
        ir_emit::emit_exp({
            .type = task_type::EXPRESSION,
            .mode = exp_mode::READ,
            .func = &ir_emit::emit_exp,
            .step = 0,
            .exp  = e->left,
        });

        return;
    }
    case 1: {
        auto ssa_left = b->ssas->peek();
        assert(t.arg1.ssa_before != NULL);
        assert(t.arg1.ssa_before != ssa_left);

        push_task({
            .type            = task_type::EXPRESSION,
            .mode            = t.mode,
            .func            = &ir_emit::emit_exp_binary,
            .step            = 2,
            .exp             = t.exp,
            .arg1.ssa_before = ssa_left,
        });
        ir_emit::emit_exp({
            .type = task_type::EXPRESSION,
            .mode = exp_mode::READ,
            .func = &ir_emit::emit_exp,
            .step = 0,
            .exp  = e->right,
        });
        return;
    }
    case 2: {
        auto ssa_left  = t.arg1.ssa_before;
        auto ssa_right = b->ssas->peek();

        assert(ssa_left != NULL);
        assert(t.arg1.ssa_before != NULL);
        assert(t.arg1.ssa_before != ssa_right);

        switch (e->op) {
        case ast::op::add:
            b->add(ssa_left, ssa_right);
            break;
        case ast::op::equals:
            b->sub(ssa_left, ssa_right);
            break;
        default:
            log::warnf("Binary op not handled (%i)", e->op);
            break;
        }
        return;
    }
    }
}

void ir_emit::emit_exp_ident(emit_task t)
{
    log::tracef("emit_exp_ident [%s] (%i)", t.mode == exp_mode::READ ? "READ" : "WRITE", t.step);

    auto s = &t.exp->ident;
    assert(t.mode != exp_mode::NOT_SET);

    if (t.mode == exp_mode::READ) {
        log::noimpl();
    } else {
        assert(t.arg1.ssa_write != NULL);
        assert(!t.arg1.ssa_write->has_flag(ir::ssa_prop::no_value));

        auto sym = local_symbols->resolve(s->name);
        assert(sym != NOT_RESOLVED);
        sym->push_ssa(t.arg1.ssa_write);
    }
}

void ir_emit::emit_exp_literal(emit_task t)
{
    log::tracef("emit_exp_literal (%i)", t.step);
    assert(t.mode == exp_mode::READ);

    b->literal(t.exp->literal.val_u64);
}

void ir_emit::emit_exp_unary(emit_task t)
{
    log::tracef("emit_exp_unary (%i)", t.step);
    log::noimpl();
}