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
char *args_reg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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
                printf("  je .Lend%d\n", local_label);
            }

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
    case ND_ADDR:
        gen_lval(node->lhs);
        return;
    case ND_IND_REF:
        compile_node(node->lhs);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    }


    if (node->kind == ND_FUNC_CALL) {
        int local_label = label_num++;
        char *name = calloc(node->func_name_len + 1, sizeof(char));
        memcpy(name, node->func_name, node->func_name_len);
        int now_argc = 0;
        for (now_argc = 0; now_argc < node->func_argc; now_argc++) {
            compile_node(node->func_args[now_argc]);
        }

        for (int i = now_argc - 1; i >= 0; i--) {
            printf("  pop %s\n", args_reg[i]);
        }

        // Adjust RSP so that it is a multiple of 16bytes.
        printf("  mov rax, rsp\n");
        printf("  and rax, 15\n");
        printf("  jz .Lcall_normal%d\n", local_label);

        // Not multiple of 16bytes
        printf("  sub rsp, 8\n");
        printf("  mov rax, %d\n", node->func_args); // Number of arguments
        printf("  call %s\n", name);
        printf("  add rsp, 8\n");
        printf("  jmp .Lcall_end%d\n", local_label);

        // Normal call
        printf(".Lcall_normal%d:\n", local_label);
        printf("  mov rax, %d\n", node->func_args); // Number of arguments
        printf("  call %s\n", name);

        printf(".Lcall_end%d:\n", local_label);
        printf("  push rax\n");
        return;
    }

    if (node->kind == ND_BLOCK) {
        for (Node *now_stmt = node->stmt_next; now_stmt; now_stmt = now_stmt->stmt_next) {
            compile_node(now_stmt);
        }
        return;
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

void codegen(Function *start_fn) {
    // Print the initial part of the assembly
    printf(".intel_syntax noprefix\n");

    for (Function *now_fn = start_fn; now_fn; now_fn = now_fn->next) {
        char *name = calloc(now_fn->name_len + 1, sizeof(char));
        memcpy(name, now_fn->name, now_fn->name_len);
        printf(".global %s\n", name);
        printf("%s:\n", name);

        // Prologue
        // Allocate 26 variables.
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", now_fn->variables_num * 8);

        for (int i = 0; i < now_fn->func_argc; i++) {
            printf("  mov rax, rbp\n");
            printf("  sub rax, %d\n", now_fn->func_args[i]->offset);
            printf("  mov [rax], %s\n", args_reg[i]);
        }

        compile_node(now_fn->node);

        // Epilogue
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret \n");
    }
    
}
