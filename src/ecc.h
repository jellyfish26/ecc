#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// tokenize.c

typedef enum {
    TK_SYMBOL, // Symbol
    TK_IDENT, // Identify
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
    int len; // Length of token string
};

extern Token *now_token;
extern char *user_input;

void errorf(char *format, ...);

// Extend errorf (ex point location)
void errorf_at(char *loc, char *format, ...);

bool move_symbol(char *op);
void move_expect_symbol(char *op);
int move_expect_number();
Token *move_ident();

bool is_eof();

Token *tokenize(char *str_p);

// parse.c

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ, // ==
    ND_NEQ, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_ASSIGN, // =
    ND_LVAR, // Local Variable
    ND_INT, // Integer
} NodeKind;

typedef struct Node Node;

// Type Node
struct Node {
    NodeKind kind; // Type of NodeKind
    Node *lhs; // left
    Node *rhs; // right
    int val; // Integer value if NodeKind is ND_INT
    int offset; // offset value if NodeKind is ND_LVAL
};

void program();
extern Node *code[100];

// codegen.c

void codegen();