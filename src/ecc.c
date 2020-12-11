#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>

typedef enum {
    TK_SYMBOL, // Symbol
    TK_INT, // Integer value
    TK_EOF, // End Of File
} TokenKind;

typedef struct Token Token;

// Type Token
struct Token {
    TokenKind kind; // Type of TokenKind
    Token *next; // Next Token
    int val; // Numerical value if TokenKind is TK_INT
    char *str; // Token String (etc operator)
};

Token *now_token;

void errorf(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool move_symbol(char op) {
    if (now_token->kind != TK_SYMBOL || now_token->str[0] != op) {
        return false;
    }
    now_token = now_token->next;
    return true;
}

void move_expect_symbol(char op) {
    if (now_token->kind != TK_SYMBOL || now_token->str[0] != op) {
        errorf("[ NG ] Invalid character '%c'", op);
    }
    now_token = now_token->next;
}

int move_expect_number() {
    if (now_token->kind != TK_INT) {
        errorf("[ NG ] Not a number");
    }
    int val = now_token->val;
    now_token = now_token->next;
    return val;
}

bool is_eof() {
    return (now_token->kind == TK_EOF);
}

Token *new_token(TokenKind kind, Token *current, char *str) {
    Token *ret = calloc(1, sizeof(Token));
    ret->kind = kind;
    ret->str = str;
    current->next = ret;
    return ret;
}

Token *tokenize(char *str_p) {
    Token head;
    head.next = NULL;
    Token *current = &head;

    while (*str_p) {
        if (isspace(*str_p)) {
            str_p++;
            continue;
        }

        if (*str_p == '+' || *str_p == '-') {
            current = new_token(TK_SYMBOL, current, str_p++);
            continue;
        }

        if (isdigit(*str_p)) {
            current = new_token(TK_INT, current, str_p);
            current->val = strtol(str_p, &str_p, 10);
            continue;
        }

        errorf("Cannot tokenize");
    }

    new_token(TK_EOF, current, str_p);
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments");
    }

    now_token = tokenize(argv[1]);

    // Print the initial part of the assembly
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // Load first value
    printf("  mov rax, %ld\n", move_expect_number());

    while (!is_eof()) {
        if (move_symbol('+')) {
            printf("  add rax, %d\n", move_expect_number());
            continue;
        }

        move_expect_symbol('-');
        printf("  sub rax, %d\n", move_expect_number());
    }

    printf("  ret\n");
    return 0;
}