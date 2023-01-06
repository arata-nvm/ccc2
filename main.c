#include <stdio.h>

int main()
{
  int number;

  scanf("%d", &number);
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  printf("  movs r0, #%d\n", number);
  printf("  bx lr\n");

  return 0;
}
