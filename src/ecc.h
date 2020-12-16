#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// tokenize.c

typedef enum {
    TK_SYMBOL, // Symbol
    TK_IDENT,  // Identify
    TK_INT,    // Integer value
    TK_RETURN, // "return" statement
    TK_IF,     // "if" statement
    TK_ELSE,   // "else" statement
    TK_FOR,    // "for" statement
    TK_WHILE,  // "while" statement
    TK_TYPE,   // Variable types
    TK_EOF,    // End Of File
} TokenKind;

typedef struct Token Token;

// Type Token
struct Token {
    TokenKind kind; // Type of TokenKind
    Token *next;    // Next Token
    int val;        // Integer value if TokenKind is TK_INT
    char *str;      // Token String (etc operator)
    int len;        // Length of token string
};

extern Token *now_token;
extern char *user_input;

void errorf(char *format, ...);

// Extend errorf (ex point location)
void errorf_at(char *loc, char *format, ...);

bool move_symbol(char *op);
void move_expect_symbol(char *op);
int move_expect_number();
Token *move_any_tokenkind(TokenKind kind);

bool is_eof();

Token *tokenize(char *str_p);

typedef struct Type Type;
typedef struct Node Node;
typedef struct LVar LVar;
typedef struct Function Function;

// type.c

typedef enum {
    TY_INT,  // Integer
    TY_PTR,  // Pointer
} TypeKind;

struct Type {
    TypeKind kind;
    Type *ptr_to;
};

Type *int_type();
Type *pointer_type(Type *target);
void init_type_function(Function *target);

// parse.c

typedef enum {
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_EQ,        // ==
    ND_NEQ,       // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_ASSIGN,    // =
    ND_ADDR,      // &
    ND_IND_REF,   // *
    ND_LVAR,      // Local variable
    ND_BLOCK,     // any statements
    ND_RETURN,    // "return" statement
    ND_IF,        // "if" statement
    ND_ELSE,      // "else" statement
    ND_FOR,       // "for" statement
    ND_WHILE,     // "while" statement
    ND_FUNC,      // Function definition
    ND_FUNC_CALL, // Function call
    ND_INT,       // Integer
} NodeKind;



// Type Node
struct Node {
    NodeKind kind; // Type of NodeKind
    Node *lhs;     // Left node
    Node *rhs;     // Right node

    // "if" statement
    Node *judge_if;
    Node *exec_if;
    Node *stmt_else;

    // "for" or "while" statement
    Node *init_for;
    Node *judge_for;
    Node *repeat_for;
    Node *stmt_for;

    // Block statements
    Node *stmt_next;

    Type *type;            // Type (ex int, pointer)
    LVar *local_variable;  // Contents of variable
    int val;               // Integer value if NodeKind is ND_INT

    char *func_name;     // Function call name
    int func_name_len;   // Length of function call name
    int func_argc;       // Number of function call arguments
    Node *func_args[6];  // Contents of function call arguments
};

struct LVar {
    LVar *next; // Next variable (NULL if no variable)
    Type *type; // Variable type
    char *name; // Variable name
    int len;    // Length of name
    int offset; // Offset from RBP (Base pointer)
};

struct Function {
    Function *next;        // Next function
    Node *node;            // Node tip
    char *name;            // Function name
    int name_len;          // Length of function name
    LVar *local_variables; // Local variables
    int variables_num;     // Number of local variable
    int func_argc;         // Number of function arguments
    LVar *func_args[6];    // Contents of function arguments
};

Function *program();

// codegen.c

void codegen();