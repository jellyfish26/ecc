#include "ecc.h"

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        errorf("Left side value of assignment is not a variable");
    }

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

int label_num = 0;

void compile_node(Node *node) {
    if (node->kind == ND_INT) {
        printf("  push %d\n", node->val);
        return;
    }

    switch(node->kind) {
    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        compile_node(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    case ND_RETURN:
        compile_node(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case ND_IF:
        {
            int local_label = label_num++;
            compile_node(node->judge_if);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lelse%d\n", local_label);

            // "if" is true
            compile_node(node->exec_if);
            printf("  jmp .Lend%d\n", local_label);

            // "else" statement
            printf(".Lelse%d:\n", local_label);
            if (node->stmt_else) {
                compile_node(node->stmt_else);
                printf("  jmp .Lend%d\n", local_label);
            }

            // finally
            printf(".Lend%d:\n", local_label);
            return;
        }
    case ND_WHILE:
    case ND_FOR:
        {
            int local_label = label_num++;
            if (node->init_for) {
                compile_node(node->init_for);
            }

            printf(".Lbegin%d:\n", local_label);

            // "for" judge expr
            if (node->judge_for) {
                compile_node(node->judge_for);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
            }

            printf("  je .Lend%d\n", local_label);

            // "for" innner statement
            compile_node(node->stmt_for);

            // repeate
            if (node->repeat_for) {
                compile_node(node->repeat_for);
            }

            // finally
            printf("  jmp .Lbegin%d\n", local_label);
            printf(".Lend%d:\n", local_label);
            return;
        }
    }

    compile_node(node->lhs);
    compile_node(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
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

void codegen() {
    // Print the initial part of the assembly
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");


    // Prologue
    // Allocate 26 variables.
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        compile_node(code[i]);

        // Pop result value 
        printf("  pop rax\n");
    }

    // Epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret \n");
}