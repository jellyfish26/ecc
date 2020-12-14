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

// Warning: This function only checks the TokenKind.
Token *move_any_tokenkind(TokenKind  kind) {
    if (now_token->kind != kind) {
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

bool is_useable_char(char c) {
    bool ret = false;
    ret |= ('a' <= c && c <= 'z');
    ret |= ('A' <= c && c <= 'Z');
    ret |= (c == '_');
    return ret;
}

bool is_identify_char(char c) {
    return ('0' <= c && c <= '9') || is_useable_char(c);
}

Token *tokenize(char *str_p) {
    Token head;
    head.next = NULL;
    Token *current = &head;
    user_input = str_p;

    char *permit_symbol[] = {
        "==", "!=", ">=", "<=",
        "+", "-", "*", "/", "(", ")", "<", ">", "=", "!", ";", "{", "}", ","
    };

    while (*str_p) {
        if (isspace(*str_p)) {
            str_p++;
            continue;
        }

        bool check = false;
        for (int i = 0; i < 18; i++) {
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

        if (isdigit(*str_p)) {
            current = new_token(TK_INT, current, str_p, 0);
            char *before = str_p;
            current->val = strtol(str_p, &str_p, 10);
            current->len = str_p - before;
            continue;
        }

        // "return" statement
        if (strncmp(str_p, "return", 6) == 0 && !is_identify_char(str_p[6])) {
            current = new_token(TK_RETURN, current, str_p, 6);
            str_p += 6;
            continue;
        }

        // "if" statement
        if (strncmp(str_p, "if", 2) == 0) {
            current = new_token(TK_IF, current, str_p, 2);
            str_p += 2;
            continue;
        }

        // "else" statement
        if (strncmp(str_p, "else", 4) == 0) {
            current = new_token(TK_ELSE, current, str_p, 4);
            str_p += 4;
            continue;
        }

        // "for" statement
        if (strncmp(str_p, "for", 3) == 0) {
            current = new_token(TK_FOR, current, str_p, 3);
            str_p += 3;
            continue;
        }

        // "while" statement
        if (strncmp(str_p, "while", 5) == 0) {
            current = new_token(TK_WHILE, current, str_p, 5);
            str_p += 5;
            continue;
        }

        if (is_identify_char(*str_p)) {
            char *start = str_p;
            while (is_identify_char(*str_p)) {
                str_p++;
            }
            int len = str_p - start;
            current = new_token(TK_IDENT, current, start, len);
            continue;
        }

        errorf("Cannot tokenize");
    }

    new_token(TK_EOF, current, str_p, 0);
    return head.next;
}