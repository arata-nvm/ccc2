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

assert 0 "0"
assert 42 "42"
assert 3 "1+2"
assert 13 "1+10+2"
assert 1 "2-1"
assert 2 "4-3+2-1"
assert 13 "1+3*4"
assert 27 "4*6+6/2"
assert 1 "3%2"
assert 6 "+1*-2*-3"
assert 14 "1*2+3*4"
assert 20 "1*(2+3)*4"
assert 0 "0<0"
assert 1 "0<1"
assert 0 "1<0"
assert 1 "0<=0"
assert 1 "0<=1"
assert 0 "1<=0"
assert 0 "0>0"
assert 0 "0>1"
assert 1 "1>0"
assert 1 "0>=0"
assert 0 "0>=1"
assert 1 "1>=0"
assert 1 "0==0"
assert 0 "0==1"
assert 0 "0!=0"
assert 1 "0!=1"

echo "OK"
