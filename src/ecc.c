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
    int val; // Integer value if TokenKind is TK_INT
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

char *user_input;

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

bool move_symbol(char op) {
    if (now_token->kind != TK_SYMBOL || now_token->str[0] != op) {
        return false;
    }
    now_token = now_token->next;
    return true;
}

void move_expect_symbol(char op) {
    if (now_token->kind != TK_SYMBOL || now_token->str[0] != op) {
        errorf_at(now_token->str, "Invalid character '%c'", op);
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

    char permit_symbol[] = {'+', '-', '*', '/', '(', ')'};

    while (*str_p) {
        if (isspace(*str_p)) {
            str_p++;
            continue;
        }

        bool check = false;
        for (int i = 0; i < sizeof(permit_symbol) / sizeof(char); i++) {
            if (*str_p == permit_symbol[i]) {
                current = new_token(TK_SYMBOL, current, str_p++);
                check = true;
                break;
            }
        }

        if (check) {
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

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_INT, // Integer
} NodeKind;

typedef struct Node Node;

// Type Node
struct Node {
    NodeKind kind; // Type of NodeKind
    Node *lhs; // left
    Node *rhs; // right
    int val; // Integer value if NodeKind is ND_INT
};

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
Node *expr();
Node *mul();
Node *primary();

Node *expr() {
    Node *ret = mul();

    while (1) {
        if (move_symbol('+')) {
            ret = new_node(ND_ADD, ret, mul());
        } else if (move_symbol('-')) {
            ret = new_node(ND_SUB, ret, mul());
        } else {
            return ret;
        }
    }
}

Node *mul() {
    Node *ret = primary();

    while (1) {
        if (move_symbol('*')) {
            ret = new_node(ND_MUL, ret, primary());
        } else if (move_symbol('/')) {
            ret = new_node(ND_DIV, ret, primary());
        } else {
            return ret;
        }
    }
}

Node *primary() {
    if (move_symbol('(')) {
        Node *ret = expr();
        move_expect_symbol(')');
        return ret;
    }

    return new_node_int(move_expect_number());
}

void compile_node(Node *now_node) {
    if (now_node->kind == ND_INT) {
        printf("  push %d\n", now_node->val);
        return;
    }

    compile_node(now_node->lhs);
    compile_node(now_node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (now_node->kind) {
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    }
    
    printf("  push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments");
    }

    user_input = argv[1];
    now_token = tokenize(argv[1]);

    // Print the initial part of the assembly
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    Node *node = expr();
    compile_node(node);

    printf("  pop rax\n");
    printf("  ret\n");

    return 0;
}