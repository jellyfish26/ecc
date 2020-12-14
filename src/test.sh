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

assert $((2+5)) "return 2+5;"
assert $((3+7-10+4)) "return 3+7-10+4;"
assert $((10+6*2)) "return 10+6*2;"
assert $((2*5+4*8)) "return 2*5+4*8;"
assert $(((3+5)*2)) "return (3+5)*2;"
assert $(((3+5)/2)) "return (3+5)/2;"
assert $((-2+5)) "return -2+5;"
assert $((-2*-3)) "return -2*-3;"
assert $((-(3+5)+10)) "return -(3+5)+10;"
assert $((- - 3 + 5)) "return - - 3 + 5;"
assert $((- 3 - -5)) "return - 3 - -5;"

assert $((1 == 1)) "return 1 == 1;"
assert $((1 <= 1)) "return 1 <= 1;"
assert $((1 >= 1)) "return 1 >= 1;"
assert $((1 > 0)) "return 1 > 0;"
assert $((1 < 0)) "return 1 < 0;"
assert $((0 > 1)) "return 0 > 1;"
assert $((0 < 1)) "return 0 < 1;"
assert $((1 >= 0)) "return 1 >= 0;"
assert $((1 <= 0)) "return 1 <= 0;"
assert $((0 >= 1)) "return 0 >= 1;"
assert $((0 <= 1)) "return 0 <= 1;"
assert $((0 != 1)) "return 0 != 1;"
assert $(((0 == 0) + (0 < 1))) "return (0 == 0) + (0 < 1);"

assert 1 "a = 1; return a;"
assert 3 "a = 1; b = 2; return a + b;"
assert 20 "a = 1 + 4; b = 4; z = a - b; return a * b / z;"
assert 12 "foo = 3; bar = 4; return foo * bar;"
assert 3 "hoge = 4; fuga = 3; return fuga; return hoge;"

assert 5 "if (1 == 1) return 5; return 2;"
assert 2 "if (0 == 1) return 5; return 2;"
assert 2 "if (3 - 3) return 5; return 2;"
assert 5 "if (3 - 2) return 5; return 2;"
assert 4 "foo = 3; bar = 4; if (foo != bar) return bar; return 2;"

assert 5 "if (1 == 1) return 5; else return 2;"
assert 2 "if (0 == 1) return 5; else return 2;"
assert 2 "if (3 - 3) return 5; else return 2;"
assert 5 "if (3 - 2) return 5; else return 2;"
assert 1 "tmp = 5; if (tmp == 5) tmp = 1; else tmp = 2; return tmp;"
assert 2 "tmp = 4; if (tmp == 5) tmp = 1; else tmp = 2; return tmp;"

assert 5 "for (i = 0; i < 5; i = i + 1) tmp = 1; return i;"
assert 10 "for (i = 0;;i = i + 1) if (i == 10) return i;"
assert 20 "i = 1; for (;; i = i + 1) if (i == 20) return i;"
assert 1 "i = 1; for (;;) if (i == 1) return i;"

assert 15 "i = 0; while (i < 15) i = i + 1; return i;"

assert 30 "i = 0; j = 0; while (i < 15) { i = i + 1; j = j + 2; } return j;"
assert 10 "i = 0; while(i != -1) { i = i + 1; if (i == 10) return i; }"
assert 10 "i = 0; for (;;) { i = i + 1; if (i == 10) return i; }"

assert 3 "foo(2, 1);"
assert 62 "bar(102, 40);"

echo OK