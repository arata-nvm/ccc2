#include <stdio.h>

int main()
{
  int number;

  scanf("%d", &number);
  printf(".global main\n");
  printf("main:\n");
  printf("  mov x0, #%d\n", number);
  printf("  ret\n");

  return 0;
}
