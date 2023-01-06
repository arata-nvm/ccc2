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

assert 0 "main() { return 0; }"
assert 42 "main() { return 42; }"
assert 3 "main() { return 1+2; }"
assert 13 "main() { return 1+10+2; }"
assert 1 "main() { return 2-1; }"
assert 2 "main() { return 4-3+2-1; }"
assert 13 "main() { return 1+3*4; }"
assert 27 "main() { return 4*6+6/2; }"
assert 1 "main() { return 3%2; }"
assert 6 "main() { return +1*-2*-3; }"
assert 14 "main() { return 1*2+3*4; }"
assert 20 "main() { return 1*(2+3)*4; }"
assert 0 "main() { return 0<0; }"
assert 1 "main() { return 0<1; }"
assert 0 "main() { return 1<0; }"
assert 1 "main() { return 0<=0; }"
assert 1 "main() { return 0<=1; }"
assert 0 "main() { return 1<=0; }"
assert 0 "main() { return 0>0; }"
assert 0 "main() { return 0>1; }"
assert 1 "main() { return 1>0; }"
assert 1 "main() { return 0>=0; }"
assert 0 "main() { return 0>=1; }"
assert 1 "main() { return 1>=0; }"
assert 1 "main() { return 0==0; }"
assert 0 "main() { return 0==1; }"
assert 0 "main() { return 0!=0; }"
assert 1 "main() { return 0!=1; }"
assert 1 "main() { x=1; return x; }"
assert 3 "main() { a=1; b=2; return a+b; }"
assert 1 "main() { return 1; return 2; }"
assert 1 "main() { if (0==0) { return 1; } return 2; }"
assert 2 "main() { if (0==1) { return 1; } return 2; }"
assert 1 "main() { if (0==0) { return 1; } else { return 2; } return 3; }"
assert 2 "main() { if (0==1) { return 1; } else { return 2; } return 3; }"
assert 10 "main() { i = 0; while (i < 10) { i = i + 1; } return i; }"
assert 55 "main() { sum = 0; for (i = 0; i <= 10; i = i + 1) { sum = sum + i; } return sum; }"
assert 1 "main() { {} return 1; }"
assert 1 "test() { return 1; } main() { return test(); }"
assert 1 "test(a) { return a; } main() { return test(1); }"
assert 3 "test(a, b) { return a + b; } main() { return test(1, 2); }"
assert 34 "fib(n) { if (n <= 2) { return 1; } else { return fib(n - 1) + fib(n - 2); } } main() { return fib(9); }"
assert 1 "main() { a = 1; b = &a; return *b; }"
assert 2 "main() { a = 1; b = &a; *b = 2; return a; }"

echo "OK"
