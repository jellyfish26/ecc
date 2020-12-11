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

assert $((2+5)) "2+5"
assert $((3+7-10+4)) "3+7-10+4"
assert $((10+6*2)) "10+6*2"
assert $((2*5+4*8)) "2*5+4*8"
assert $(((3+5)*2)) "(3+5)*2"
assert $(((3+5)/2)) "(3+5)/2"

echo OK