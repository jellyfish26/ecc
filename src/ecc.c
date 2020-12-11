#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments");
    }

    // Print the initial part of the assembly
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    char *str_p = argv[1];
    printf("  mov rax, %ld\n", strtol(str_p, &str_p, 10));

    while (*str_p) {
        if (*str_p == '+') {
            str_p++;
            printf("  add rax, %ld\n", strtol(str_p, &str_p, 10));
            continue;
        }

        if (*str_p == '-') {
            str_p++;
            printf("  sub rax, %ld\n", strtol(str_p, &str_p, 10));
            continue;
        }

        fprintf(stderr, "Unexptcted char: %c\n", *str_p);
        return 1;
    }

    printf("  ret\n");
    return 0;
}