#include "ecc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments");
    }

    now_token = tokenize(argv[1]);
    Function *start_fn = program();
    init_type_function(start_fn);

    codegen(start_fn);

    return 0;
}