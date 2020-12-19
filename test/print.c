#include <stdio.h>
#include <stdlib.h>

int foo(int i, int j) {
    printf("%d\n", i + j);
    return i + j;
}

int bar(int i, int j) {
    printf("%d\n", i - j);
    return i - j;
}

void output(long a) {
    printf("%ld\n", a);
}

void alloc4(long **x, long a, long b, long c, long d) {
    *x = calloc(4, sizeof(long));
    long *y = *x;
    y[0] = a;
    y[1] = b;
    y[2] = c;
    y[3] = d;
}

void alloc() {
    long *x;
    printf("%x\n", x);
    alloc4(&x, 1, 2, 4, 8);
    printf("%x\n", *(x + 1));
}

