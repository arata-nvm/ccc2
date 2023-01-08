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

assert 0 "int main() { return 0; }"
assert 42 "int main() { return 42; }"
assert 3 "int main() { return 1+2; }"
assert 13 "int main() { return 1+10+2; }"
assert 1 "int main() { return 2-1; }"
assert 2 "int main() { return 4-3+2-1; }"
assert 13 "int main() { return 1+3*4; }"
assert 27 "int main() { return 4*6+6/2; }"
assert 1 "int main() { return 3%2; }"
assert 6 "int main() { return +1*-2*-3; }"
assert 14 "int main() { return 1*2+3*4; }"
assert 20 "int main() { return 1*(2+3)*4; }"
assert 0 "int main() { return 0<0; }"
assert 1 "int main() { return 0<1; }"
assert 0 "int main() { return 1<0; }"
assert 1 "int main() { return 0<=0; }"
assert 1 "int main() { return 0<=1; }"
assert 0 "int main() { return 1<=0; }"
assert 0 "int main() { return 0>0; }"
assert 0 "int main() { return 0>1; }"
assert 1 "int main() { return 1>0; }"
assert 1 "int main() { return 0>=0; }"
assert 0 "int main() { return 0>=1; }"
assert 1 "int main() { return 1>=0; }"
assert 1 "int main() { return 0==0; }"
assert 0 "int main() { return 0==1; }"
assert 0 "int main() { return 0!=0; }"
assert 1 "int main() { return 0!=1; }"
assert 1 "int main() { int x = 1; return x; }"
assert 3 "int main() { int a = 1; int b = 2; return a+b; }"
assert 5 "int main() { int a = 1; int b = 2; a = 3; return a+b; }"
assert 1 "int main() { return 1; return 2; }"
assert 1 "int main() { if (0==0) { return 1; } return 2; }"
assert 2 "int main() { if (0==1) { return 1; } return 2; }"
assert 1 "int main() { if (0==0) { return 1; } else { return 2; } return 3; }"
assert 2 "int main() { if (0==1) { return 1; } else { return 2; } return 3; }"
assert 10 "int main() { int i = 0; while (i < 10) { i = i + 1; } return i; }"
assert 55 "int main() { int sum = 0; for (int i = 0; i <= 10; i = i + 1) { sum = sum + i; } return sum; }"
assert 1 "int main() { {} return 1; }"
assert 1 "int test() { return 1; } int main() { return test(); }"
assert 1 "int test(int a) { return a; } int main() { return test(1); }"
assert 3 "int test(int a, int b) { return a + b; } int main() { return test(1, 2); }"
assert 34 "int fib(int n) { if (n <= 2) { return 1; } else { return fib(n - 1) + fib(n - 2); } } int main() { return fib(9); }"
assert 1 "int main() { int a = 1; int *b = &a; return *b; }"
assert 2 "int main() { int a = 1; int *b = &a; *b = 2; return a; }"
assert 2 "int main() { int a = 1; int *b = &a; int **c = &b; **c = 2; return a; }"
assert 2 "int main() { int a = 1; int b = 2; int *p = &a; p = p + 1; return *p; }"
assert 2 "int main() { int a = 1; int b = 2; int *p = &a; p = 1 + p; return *p; }"
assert 1 "int main() { int a = 1; int b = 2; int *p = &b; p = p - 1; return *p; }"
assert 1 "int main() { int a = 1; int b = 2; int *p1 = &a; int *p2 = &b; return p2 - p1; }"
assert 4 "int main() { int a; return sizeof(a); }"
assert 8 "int main() { int *a; return sizeof(a); }"

echo "OK"
