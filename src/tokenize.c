#include "ecc.h"

Token *now_token;
char *user_input;

void errorf(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Extend errorf (ex point location)
void errorf_at(char *loc, char *format, ...) {
    va_list ap;
    va_start(ap, format);

    int pos = loc - user_input;
    fprintf(stderr, "\e[31mSyntax Error\n");
    fprintf(stderr, "\e[m%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool move_symbol(char *op) {
    if (now_token->kind != TK_SYMBOL ||
        now_token->len != strlen(op) ||
        memcmp(now_token->str, op, now_token->len))
    {
        return false;
    }
    now_token = now_token->next;
    return true;
}

void move_expect_symbol(char *op) {
    if (now_token->kind != TK_SYMBOL ||
        now_token->len != strlen(op) ||
        memcmp(now_token->str, op, now_token->len))
    {
        errorf_at(now_token->str, "Invalid character '%s'", op);
    }
    now_token = now_token->next;
}

int move_expect_number() {
    if (now_token->kind != TK_INT) {
        errorf_at(now_token->str, "Not a number");
    }
    int val = now_token->val;
    now_token = now_token->next;
    return val;
}

Token *move_ident() {
    if (now_token->kind != TK_IDENT) {
        return NULL;
    }
    Token *ret = now_token;
    now_token = now_token->next;
    return ret;
}

bool is_eof() {
    return (now_token->kind == TK_EOF);
}

Token *new_token(TokenKind kind, Token *current, char *str, int len) {
    Token *ret = calloc(1, sizeof(Token));
    ret->kind = kind;
    ret->str = str;
    ret->len = len;
    current->next = ret;
    return ret;
}

Token *tokenize(char *str_p) {
    Token head;
    head.next = NULL;
    Token *current = &head;
    user_input = str_p;

    char *permit_symbol[] = {
        "==", "!=", ">=", "<=",
        "+", "-", "*", "/", "(", ")", "<", ">", "=", "!", ";"
    };

    while (*str_p) {
        if (isspace(*str_p)) {
            str_p++;
            continue;
        }

        bool check = false;
        for (int i = 0; i < 15; i++) {
            int len = strlen(permit_symbol[i]);
            if (memcmp(str_p, permit_symbol[i], len) == 0) {
                current = new_token(TK_SYMBOL, current, str_p, len);
                str_p += len;
                check = true;
                break;
            }
        }

        if (check) {
            continue;
        }

        if ('a' <= *str_p && *str_p <= 'z') {
            current = new_token(TK_IDENT, current, str_p++, 1);
            current->len = 1;
            continue;
        }

        if (isdigit(*str_p)) {
            current = new_token(TK_INT, current, str_p, 0);
            char *before = str_p;
            current->val = strtol(str_p, &str_p, 10);
            current->len = str_p - before;
            continue;
        }

        errorf("Cannot tokenize");
    }

    new_token(TK_EOF, current, str_p, 0);
    return head.next;
}