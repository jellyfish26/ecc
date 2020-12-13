#include "ecc.h"

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
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NEQ:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    }
    
    printf("  push rax\n");
}

void codegen(Node *now_node) {
    // Print the initial part of the assembly
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    compile_node(now_node);

    printf("  pop rax\n");
    printf("  ret\n");
}