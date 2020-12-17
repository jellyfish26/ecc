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
LVar *add_lvar(Token *target, Type *type) {
    LVar *local = find_lvar(target);
    if (local) {
        return local;
    }
    now_function->variables_size += type->type_size;
    local = calloc(1, sizeof(LVar));
    local->next = now_function->local_variables;
    local->type = type;
    local->name = target->str;
    local->len = target->len;
    now_function->local_variables = local;
    return local;
}


// basetype = "int" "*"*
Type *basetype() {
    Token *tmp = move_any_tokenkind(TK_TYPE);
    if (!tmp) {
        return NULL;
    }
    Type *ret = int_type();
    while (move_symbol("*")) {
        ret = pointer_type(ret);
    }
    return ret;
}

// program  = function*
// function = basetype ident "(" params? ")" statement
// params   = param ("," param)*
// param    = basetype ident
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
        Type *func_type = basetype();
        if (!func_type) {
            errorf_at(now_token->str, "Invalid type");
        }
        Token *token = move_any_tokenkind(TK_IDENT);
        if (token) {
            now_function->name = token->str;
            now_function->name_len = token->len;
            move_expect_symbol("(");

            if (!move_symbol(")")) {
                while (true) {
                    Type *var_type = basetype();
                    if (!var_type) {
                        errorf_at(now_token->str, "Invalid type");
                    }
                    Token *local = move_any_tokenkind(TK_IDENT);
                    if (local) {
                        LVar *tmp = add_lvar(local, var_type);
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

            // setting offset
            int now_offset = 0;
            for (LVar *now_var = now_function->local_variables; now_var; now_var = now_var->next) {
                now_offset += now_var->type->type_size;
                now_var->offset = now_offset;
            }
        } else {
            errorf_at(token->str, "Statement must always start with a function");
        }
    }
    return ret;
}

// statement = expr? ";"
//           | "{" statement* "}"
//           | "if" "(" expr ")" statement ("else" statement)?
//           | "for" "(" expr? "," expr? "," expr? ")" statement
//           | "while" "(" expr ")" statement
//           | "return" expr ";"
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

// expr = assign
Node *expr() {
    return assign();
}


// assign = equality ("=" assign)?
Node *assign() {
    Node *ret = equality();

    if (move_symbol("=")) {
        ret = new_node(ND_ASSIGN, ret, assign());
    }
    return ret;
}

// equality = relational ("==" relational | "!=" relational)?
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

// relational = add ("<=" add | ">=" add | "<" add | ">" add)?
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

// add = mul ("+" mul | "-" mul)?
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

// mul = unary ("*" unary | "/" unary)?
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

// unary = primary
//       | "sizeof" unary
//       | ("+" unary | "-" unary | "&" unary | "*" unary)
Node *unary() {
    if (move_symbol("+")) {
        return unary();
    } else if (move_symbol("-")) {
        return new_node(ND_SUB, new_node_int(0), unary());
    } else if (move_symbol("&")) {
        return new_node(ND_ADDR, unary(), NULL);
    } else if (move_symbol("*")) {
        return new_node(ND_IND_REF, unary(), NULL);
    }

    Token *token = move_any_tokenkind(TK_SIZEOF);
    if (token) {
        if (memcmp(token->str, "sizeof", 6) == 0) {
            return new_node(ND_SIZEOF, unary(), NULL);
        }
    }

    return primary();
}

// primary = "(" expr ")"
//         | basetype ident ("[" num "]")* -> not already
//         | ident -> already exists
//         | ident "(" params? ")"
//         | num
// params  = param ("," param)*
// param   = ident
Node *primary() {
    Node *ret;
    if (move_symbol("(")) {
        ret = expr();
        move_expect_symbol(")");
        return ret;
    }

    Type *var_type = basetype();
    if (var_type) {
        Token *token = move_any_tokenkind(TK_IDENT);

        if (!token) {
            errorf_at(now_token->str, "Not identfy");
        }

        ret = new_node(ND_LVAR, NULL, NULL);

        // About Array
        typedef struct ArraySize ArraySize;

        struct ArraySize {
            int array_size;
            ArraySize *next;
        };
        ArraySize *tmp = NULL;
        while (move_symbol("[")) {
            ArraySize *now = calloc(1, sizeof(ArraySize));
            now->array_size = move_expect_number();
            if (now->array_size <= 0) {
                errorf_at(now_token->str, "Invalid array size");
            }

            if (tmp) {
                now->next = tmp;
            }
            tmp = now;
            move_expect_symbol("]");
        }

        for (; tmp; tmp = tmp->next) {
            var_type = array_type(var_type, tmp->array_size);
        }

        LVar *result = find_lvar(token);
        if (!result) {
            result = add_lvar(token, var_type);
        } else {
            errorf_at(token->str, "This variable already exists");
        }
        ret->local_variable = result;
        return ret;
    }

    Token *token = move_any_tokenkind(TK_IDENT);

    if (token) {

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
             errorf_at(token->str, "This variable is not defined");
        }
        ret->local_variable = result;
        return ret;
    }
    return new_node_int(move_expect_number());
}