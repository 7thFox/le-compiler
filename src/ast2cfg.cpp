#include "ast2cfg.hpp"

typedef bool              exp_mode;
static constexpr exp_mode exp_read  = true;
static constexpr exp_mode exp_write = false;

void write_stmt(ir::builder *b, ast::stmt *s);
void write_stmt_assign(ir::builder *b, ast::stmt_assign *s);
void write_stmt_block(ir::builder *b, ast::stmt_block *s);
void write_stmt_ifs(ir::builder *b, ast::stmt_ifs *s);
void write_stmt_local_decl(ir::builder *b, ast::stmt_local_decl *s);
void write_stmt_return(ir::builder *b, ast::stmt_return *s);

void write_exp(ir::builder *b, ast::exp *e, exp_mode is_read);
void write_exp_binary(ir::builder *b, ast::exp_binary *e, exp_mode is_read);
void write_exp_ident(ir::builder *b, ast::exp_ident *e, exp_mode is_read);
void write_exp_literal(ir::builder *b, ast::exp_literal *e, exp_mode is_read);
void write_exp_unary(ir::builder *b, ast::exp_unary *e, exp_mode is_read);

bb *ast2cfg(ir::builder *b, ast::stmt *tree)
{
    assert(b->blocks->size == 0);
    assert(b->current_block == NULL);

    write_stmt(b, tree);

    return b->blocks->head;
}

void write_stmt(ir::builder *b, ast::stmt *s)
{
}

void write_stmt_DBG(ir::builder *b, ast::stmt_DBG *s)
{
    log::fatalf("not implemented");
}

void write_stmt_assign(ir::builder *b, ast::stmt_assign *s)
{
    write_exp(b, s->right, exp_read);

    ir::ssa *exp_result = b->ssas->next - 1;
    assert(exp_result >= b->ssas->head);

    assert(!exp_result->has_flag(ir::ssa_prop::no_value));

    write_exp(b, s->left, exp_write);
}

void write_stmt_block(ir::builder *b, ast::stmt_block *s)
{
}

void write_stmt_ifs(ir::builder *b, ast::stmt_ifs *s)
{
    assert(b->current_block != NULL);

    auto merge_block = b->alloc_block();

    for (size_t i = 0; i < s->count; i++) {
        bool else_not_last = b->current_block == NULL;
        assert(!else_not_last);

        auto pair = s->pairs[i];
        assert(pair->statement != NULL);

        if (pair->maybe_condition != NULL) { // if (...)
            auto ssa_before = b->ssas->next - 1;
            write_exp(b, pair->maybe_condition, exp_read);
            auto ssa_after = b->ssas->next - 1;
            assert(ssa_after > ssa_before);

            bb *when_true  = NULL;
            bb *when_false = NULL;
            b->branch(block_br::eq, &when_true, &when_false);
            b->end_block();

            b->start_block(when_true);
            write_stmt(b, pair->statement);
            b->unconditional_branch(merge_block);
            b->end_block();

            b->start_block(when_false);
        } else { // else ...

            write_stmt(b, pair->statement);
            b->unconditional_branch(merge_block);
            b->end_block();
        }
    }

    if (b->current_block != NULL) { // no else
        b->unconditional_branch(merge_block);
        b->end_block();
    }

    b->start_block(merge_block);
}

void write_stmt_local_decl(ir::builder *b, ast::stmt_local_decl *s)
{
}

void write_stmt_return(ir::builder *b, ast::stmt_return *s)
{
}

void write_exp(ir::builder *b, ast::exp *e, exp_mode is_read)
{
}

void write_exp_binary(ir::builder *b, ast::exp_binary *e, exp_mode is_read)
{
    assert(e->left != NULL);
    assert(e->right != NULL);
    assert(e->op != ast::op::UNUSED);

    auto ssa_before = b->ssas->next - 1;
    write_exp(b, e->left, exp_read);
    auto ssa_left = b->ssas->next - 1;
    write_exp(b, e->right, exp_read);
    auto ssa_right = b->ssas->next - 1;

    assert(ssa_before < ssa_left);
    assert(ssa_left < ssa_right);

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
}

void write_exp_ident(ir::builder *b, ast::exp_ident *e, exp_mode is_read)
{
    // assert(e->anno_symbol != NO_SYMBOL);

    // if (!is_read) {
    //     get_symbol(e->anno_symbol)->push_ssa(b->ssas->peek(), b->current_block);
    // } else {
    //     // TODO JOSH: potential reduce
    //     auto sym = get_symbol(e->anno_symbol);

    //     b->start_phi(); // TODO: If this doesn't change, update to array copy
    //     for (ir::ssa **ssa = sym->_ssas->bottom; ssa < sym->_ssas->next; ssa++) {
    //         b->push_phi(*ssa);
    //     }
    //     b->end_phi();
    // }
}

void write_exp_literal(ir::builder *b, ast::exp_literal *e, exp_mode is_read)
{
    b->literal(e->val_u64);
}

void write_exp_unary(ir::builder *b, ast::exp_unary *e, exp_mode is_read)
{
    log::fatalf("not implemented");
}