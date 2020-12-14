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
LVar *local_variables;

LVar *find_lvar(Token *find) {
    for (LVar *now_var = local_variables; now_var; now_var = now_var->next) {
        if (now_var->len == find->len && !memcmp(find->str, now_var->name, now_var->len)) {
            return now_var;
        }
    }
    return NULL;
}

void program() {
    int i = 0;
    while (!is_eof()) {
        code[i++] = statement();
    }
    code[i] = NULL;
}

Node *statement() {
    Node *ret;

    // "if" statement
    if (move_any_tokenkind(TK_IF)) {
        ret = new_node(ND_IF, NULL, NULL);
        move_expect_symbol("(");
        ret->judge_if = expr();
        move_expect_symbol(")");
        ret->exec_if = statement();
        if (move_any_tokenkind(TK_ELSE)) {
            ret->stmt_else = statement();
        } else {
            ret->stmt_else = NULL;
        }
        return ret;
    }

    // "for" statement
    if (move_any_tokenkind(TK_FOR)) {
        ret = new_node(ND_FOR, NULL, NULL);
        move_expect_symbol("(");
        if (!move_symbol(";")) {
            ret->init_for = expr();
            move_expect_symbol(";");
        }
        if (!move_symbol(";")) {
            ret->judge_for = expr();
            move_expect_symbol(";");
        }
        if (!move_symbol(")")) {
            ret->repeat_for = expr();
            move_expect_symbol(")");
        }
        ret->stmt_for = statement();
        return ret;
    }

    // "while" statement
    if (move_any_tokenkind(TK_WHILE)) {
        ret = new_node(ND_WHILE, NULL, NULL);
        move_expect_symbol("(");
        ret->judge_for = expr();
        move_expect_symbol(")");
        ret->stmt_for = statement();
        return ret;
    }

    // "return" statement
    if (move_any_tokenkind(TK_RETURN)) {
        ret = new_node(ND_RETURN, expr(), NULL);
        move_expect_symbol(";");
        return ret;
    }

    ret = expr();
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

    Token *token = move_any_tokenkind(TK_IDENT);

    if (token) {
        Node *ret = new_node(ND_LVAR, NULL, NULL);

        LVar *result = find_lvar(token);
        if (result) {
            ret->offset = result->offset;
        } else {
            result = calloc(1, sizeof(LVar));
            result->next = local_variables;
            result->name = token->str;
            result->len = token->len;
            if (!local_variables) {
                result->offset = 8;
            } else {
                result->offset = local_variables->offset + 8;
            }
            ret->offset = result->offset;
            local_variables = result;
        }
        return ret;
    }

    return new_node_int(move_expect_number());
}