#include "ecc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments");
    }

    now_token = tokenize(argv[1]);
    program();

    codegen();

    return 0;
}