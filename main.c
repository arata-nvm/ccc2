#include "error.h"
#include "tokenizer.h"
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s <file>", argv[0]);
    return 1;
  }

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    error("failed to open file");
  }

  printf(".global main\n");
  printf("main:\n");

  token_t *token = read_next_token(fp);
  if (token->type == TOKEN_NUMBER) {
    printf("  mov x0, %d\n", token->value.number);
  }

  printf("  ret\n");

  return 0;
}
