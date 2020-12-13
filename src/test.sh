#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./ecc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "[ OK ] $input => $actual"
  else
    echo "[ NG ] $input => $expected expected, but got $actual"
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

echo OK