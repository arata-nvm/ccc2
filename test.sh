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

assert 0 "return 0;"
assert 42 "return 42;"
assert 3 "return 1+2;"
assert 13 "return 1+10+2;"
assert 1 "return 2-1;"
assert 2 "return 4-3+2-1;"
assert 13 "return 1+3*4;"
assert 27 "return 4*6+6/2;"
assert 1 "return 3%2;"
assert 6 "return +1*-2*-3;"
assert 14 "return 1*2+3*4;"
assert 20 "return 1*(2+3)*4;"
assert 0 "return 0<0;"
assert 1 "return 0<1;"
assert 0 "return 1<0;"
assert 1 "return 0<=0;"
assert 1 "return 0<=1;"
assert 0 "return 1<=0;"
assert 0 "return 0>0;"
assert 0 "return 0>1;"
assert 1 "return 1>0;"
assert 1 "return 0>=0;"
assert 0 "return 0>=1;"
assert 1 "return 1>=0;"
assert 1 "return 0==0;"
assert 0 "return 0==1;"
assert 0 "return 0!=0;"
assert 1 "return 0!=1;"
assert 1 "x=1; return x;"
assert 3 "a=1; b=2; return a+b;"
assert 1 "return 1; return 2;"

echo "OK;"
