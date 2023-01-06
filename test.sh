#!/bin/bash -u

assert() {
  expected="$1"
  input="$2"

  echo "$input" > tmp.c
  ./ccc tmp.c > tmp.s
  gcc -o tmp tmp.s 
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert 3 1+2
assert 13 1+10+2

echo "OK"
