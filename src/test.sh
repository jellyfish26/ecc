#!/bin/bash
gcc -std=c11 -static -c -o ../test/print.o ../test/print.c

assert() {
  expected="$1"
  input="$2"

  ./ecc "$input" > tmp.s
  gcc -o tmp tmp.s ../test/print.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo -e "\e[32m[ OK ]\e[m $input => $actual"
  else
    echo -e "\e[31m[ NG ]\e[m $input => $expected expected, but got $actual"
    exit 1
  fi
}

assert $((2+5))       "int main() { return 2+5; }"
assert $((3+7-10+4))  "int main() { return 3+7-10+4; }"
assert $((10+6*2))    "int main() { return 10+6*2; }"
assert $((2*5+4*8))   "int main() { return 2*5+4*8; }"
assert $(((3+5)*2))   "int main() { return (3+5)*2; }"
assert $(((3+5)/2))   "int main() { return (3+5)/2; }"
assert $((-2+5))      "int main() { return -2+5; }"
assert $((-2*-3))     "int main() { return -2*-3; }"
assert $((-(3+5)+10)) "int main() { return -(3+5)+10; }"
assert $((- - 3 + 5)) "int main() { return - - 3 + 5; }"
assert $((- 3 - -5))  "int main() { return - 3 - -5; }"

assert $((1 == 1)) "int main() { return 1 == 1; }"
assert $((1 <= 1)) "int main() { return 1 <= 1; }"
assert $((1 >= 1)) "int main() { return 1 >= 1; }"
assert $((1 > 0))  "int main() { return 1 > 0; }"
assert $((1 < 0))  "int main() { return 1 < 0; }"
assert $((0 > 1))  "int main() { return 0 > 1; }"
assert $((0 < 1))  "int main() { return 0 < 1; }"
assert $((1 >= 0)) "int main() { return 1 >= 0; }"
assert $((1 <= 0)) "int main() { return 1 <= 0; }"
assert $((0 >= 1)) "int main() { return 0 >= 1; }"
assert $((0 <= 1)) "int main() { return 0 <= 1; }"
assert $((0 != 1)) "int main() { return 0 != 1; }"

assert $(((0 == 0) + (0 < 1))) "int main() {return (0 == 0) + (0 < 1); }"

assert 1  "int main() { int a; a = 1; return a; }"
assert 3  "int main() { int a = 1; int b = 2; return a + b; }"
assert 20 "int main() { int a; int b; int z; a = 1 + 4; b = 4; z = a - b; return a * b / z; }"
assert 12 "int main() { int foo; int bar; foo = 3; bar = 4; return foo * bar; }"
assert 3  "int main() { int hoge; int fuga; hoge = 4; fuga = 3; return fuga; return hoge; }"

assert 5 "int main() { if (1 == 1) return 5; return 2; }"
assert 2 "int main() { if (0 == 1) return 5; return 2; }"
assert 2 "int main() { if (3 - 3) return 5; return 2; }"
assert 5 "int main() { if (3 - 2) return 5; return 2; }"
assert 4 "int main() { int foo = 3; int bar = 4; if (foo != bar) return bar; return 2; }"

assert 5 "int main() { if (1 == 1) return 5; else return 2; }"
assert 2 "int main() { if (0 == 1) return 5; else return 2; }"
assert 2 "int main() { if (3 - 3) return 5; else return 2; }"
assert 5 "int main() { if (3 - 2) return 5; else return 2; }"
assert 1 "int main() { int tmp = 5; if (tmp == 5) tmp = 1; else tmp = 2; return tmp; }"
assert 2 "int main() { int tmp = 4; if (tmp == 5) tmp = 1; else tmp = 2; return tmp; }"

assert 5  "int main() { int ans = 0; for (int i = 0; i < 5; i = i + 1) { int tmp = 1; ans = ans + tmp; tmp = tmp + 1;} return i; }"
assert 10 "int main() { for (int i = 0;;i = i + 1) if (i == 10) return i; }"
assert 20 "int main() { int i; i = 1; for (;; i = i + 1) if (i == 20) return i; }"
assert 1  "int main() { int i = 1; for (;;) if (i == 1) return i; }"

assert 15 "int main() { int i = 0; while (i < 15) i = i + 1; return i; }"

assert 30 "int main() { int i = 0; int j = 0; while (i < 15) { i = i + 1; j = j + 2; } return j; }"
assert 10 "int main() { int i = 0; while(i != -1) { i = i + 1; if (i == 10) return i; } }"
assert 10 "int main() { int i = 0; for (;;) { i = i + 1; if (i == 10) return i; } }"

assert 3  "int main() { return foo(2, 1); }"
assert 62 "int main() { return bar(102, 40); }"

assert 5 "int main() { int a = 3; hoge(); } int hoge() { int a = 5; return a; }"
assert 8 "int main() { int a = 3; int b = hoge(); return a + b; } int hoge() { int a = 5; return a; }"

assert 6  "int main() { return hoge(2, 3); } int hoge(int a, int b) { return a * b; }"
assert 23 "int main() { int a = 3; int b = 5; return a + b + hoge(a, b); } int hoge(int a, int b) { return a * b; }"

assert 3 "int main() { int x = 3; int y = &x; return *y; } "

assert 55 "int main() { return fib(10); } int fib(int a) { if (a == 0) { return 0; } else if (a == 1) { return 1; } else { return fib(a - 2) + fib(a - 1); } } "
echo All Test Passed