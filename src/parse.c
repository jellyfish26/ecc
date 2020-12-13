#include "ecc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *ret = calloc(1, sizeof(Node));
    ret->kind = kind;
    ret->lhs = lhs;
    ret->rhs = rhs;
    return ret;
}

Node *new_node_int(int val) {
    Node *ret = calloc(1, sizeof(Node));
    ret->kind = ND_INT;
    ret->val = val;
    return ret;
}

// Prototype
void program();
Node *statement();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *code[100];

void program() {
    int i = 0;
    while (!is_eof()) {
        code[i++] = statement();
    }
    code[i] = NULL;
}

Node *statement() {
    Node *ret = expr();
    move_expect_symbol(";");
    return ret;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *ret = equality();

    if (move_symbol("=")) {
        ret = new_node(ND_ASSIGN, ret, assign());
    }
    return ret;
}

Node *equality() {
    Node *ret = relational();

    while (1) {
        if (move_symbol("==")) {
            ret = new_node(ND_EQ, ret, relational());
        } else if (move_symbol("!=")) {
            ret = new_node(ND_NEQ, ret, relational());
        } else {
            return ret;
        }
    }
}

Node *relational() {
    Node *ret = add();

    while (1) {
        if (move_symbol("<=")) {
            ret = new_node(ND_LE, ret, add());
        } else if (move_symbol(">=")) {
            ret = new_node(ND_LE, add(), ret);
        } else if (move_symbol("<")) {
            ret = new_node(ND_LT, ret, add());
        } else if (move_symbol(">")) {
            ret = new_node(ND_LT, add(), ret);
        } else {
            return ret;
        }
    }
}

Node *add() {
    Node *ret = mul();

    while (1) {
        if (move_symbol("+")) {
            ret = new_node(ND_ADD, ret, mul());
        } else if (move_symbol("-")) {
            ret = new_node(ND_SUB, ret, mul());
        } else {
            return ret;
        }
    }
}

Node *mul() {
    Node *ret = unary();

    while (1) {
        if (move_symbol("*")) {
            ret = new_node(ND_MUL, ret, unary());
        } else if (move_symbol("/")) {
            ret = new_node(ND_DIV, ret, unary());
        } else {
            return ret;
        }
    }
}

Node *unary() {
    if (move_symbol("+")) {
        return unary();
    } else if (move_symbol("-")) {
        return new_node(ND_SUB, new_node_int(0), unary());
    }

    return primary();
}

Node *primary() {
    if (move_symbol("(")) {
        Node *ret = expr();
        move_expect_symbol(")");
        return ret;
    }

    Token *token = move_ident();

    if (token) {
        Node *ret = new_node(ND_LVAR, NULL, NULL);
        ret->offset = (token->str[0] - 'a' + 1) * 8;
        return ret;
    }

    return new_node_int(move_expect_number());
}