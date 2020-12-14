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
Function *program();
Node *statement();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Function *now_function;

LVar *find_lvar(Token *find) {
    for (LVar *now_var = now_function->local_variables; now_var; now_var = now_var->next) {
        if (now_var->len == find->len && !memcmp(find->str, now_var->name, now_var->len)) {
            return now_var;
        }
    }
    return NULL;
}


// If the lvar already exists, it returns the already existing offset
LVar *add_lvar(Token *target) {
    LVar *local = find_lvar(target);
    if (local) {
        return local;
    }
    now_function->variables_num++;
    local = calloc(1, sizeof(LVar));
    local->next = now_function->local_variables;
    local->name = target->str;
    local->len = target->len;
    if (!now_function->local_variables) {
        local->offset = 8;
    } else {
        local->offset = now_function->variables_num * 8;
    }
    now_function->local_variables = local;
    return local;
}

Function *program() {
    int i = 0;
    Function *ret;
    while (!is_eof()) {
        if (now_function) {
            Function *next = calloc(1, sizeof(Function));
            now_function->next = next;
            now_function = next;
        } else {
            now_function = calloc(1, sizeof(Function));
            ret = now_function;
        }

        // Function definition
        Token *token = move_any_tokenkind(TK_IDENT);
        if (token) {
            now_function->name = token->str;
            now_function->name_len = token->len;
            move_expect_symbol("(");

            if (!move_symbol(")")) {
                while (true) {
                    Token *local = move_any_tokenkind(TK_IDENT);
                    if (local) {
                        LVar *tmp = add_lvar(local);
                        now_function->func_args[now_function->func_argc] = tmp;
                        now_function->func_argc++;
                        if (move_symbol(",")) {
                            continue;
                        }

                        if (move_symbol(")")) {
                            break;
                        }
                    } else {
                        errorf_at(now_token->str, "Not variable");
                    }
                }
            }
            now_function->node = statement();
        } else {
            errorf_at(token->str, "Statement must always start with a function");
        }
    }
    return ret;
}

Node *statement() {
    Node *ret;

    // block statement
    if (move_symbol("{")) {
        ret = new_node(ND_BLOCK, NULL, NULL);
        Node *now = NULL;
        while (!move_symbol("}")) {
            if (now) {
                Node *next_node = statement();
                now->stmt_next = next_node;
                now = next_node;
            } else {
                now = statement();
                ret->stmt_next = now;
            }
        }
        return ret;
    }

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
        Node *ret;

        // Function call
        if (move_symbol("(")) {
            ret = new_node(ND_FUNC_CALL, NULL, NULL);
            ret->func_name = token->str;
            ret->func_name_len = token->len;

            if (move_symbol(")")) {
                return ret;
            }
            
            while (true) {
                ret->func_args[ret->func_argc] = expr();
                ret->func_argc++;
                if (move_symbol(",")) {
                    continue;
                }

                if (move_symbol(")")) {
                    break;
                }
            }
            return ret;
        }

        ret = new_node(ND_LVAR, NULL, NULL);

        LVar *result = find_lvar(token);
        if (!result) {
            result = add_lvar(token);
        }
        ret->offset = result->offset;
        return ret;
    }

    return new_node_int(move_expect_number());
}