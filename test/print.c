#include <stdio.h>

int foo(int i, int j) {
    printf("%d\n", i + j);
    return i + j;
}

int bar(int i, int j) {
    printf("%d\n", i - j);
    return i - j;
}