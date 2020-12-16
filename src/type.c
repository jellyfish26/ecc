#include "ecc.h"

Type *int_type() {
    Type *ret = calloc(1, sizeof(Type));
    ret->kind = TY_INT;
    return ret;
}

Type *pointer_type(Type *target) {
    Type *ret = calloc(1, sizeof(Type));
    ret->kind = TY_PTR;
    ret->ptr_to = target;
    return ret;
}

void init_type(Node *node) {
    if (!node) {
        return;
    }

    init_type(node->lhs);
    init_type(node->rhs);
    init_type(node->judge_if);
    init_type(node->exec_if);
    init_type(node->stmt_else);
    init_type(node->init_for);
    init_type(node->judge_for);
    init_type(node->repeat_for);
    init_type(node->stmt_for);
    init_type(node->stmt_next);

    for (int i = 0; i < node->func_argc; i++) {
        init_type(node->func_args[i]);
    }

    switch(node->kind) {
    case ND_ADD:
    case ND_SUB:
    {
        if (node->rhs->type->kind == TY_PTR) {
            Node *tmp = node->rhs;
            node->rhs = node->lhs;
            node->lhs = tmp;
        }
        if (node->rhs->type->kind == TY_PTR) {
            errorf("Invalid operands.");
        }
        node->type = node->lhs->type;
        return;
    }
    case ND_ASSIGN:
        node->type = node->lhs->type;
        return;
    case ND_ADDR:
        node->type = pointer_type(node->lhs->type);
        return;
    case ND_IND_REF:
        if (node->lhs->type->kind == TY_PTR) {
            node->type = node->lhs->type->ptr_to;
        } else {
            node->type = int_type();
        }
        return;
    case ND_LVAR:
        node->type = node->local_variable->type;
        return;
    default:
        node->type = int_type();
        return;
    }
}

void init_type_function(Function *target) {
    for (Function *now_fn = target; now_fn; now_fn = now_fn->next) {
        init_type(now_fn->node);
    }
}