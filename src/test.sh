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

assert 0 0
assert 127 127
assert 255 255

echo OK