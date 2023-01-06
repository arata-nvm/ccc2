#include "error.h"
#include "parser.h"
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

  token_t *token = tokenize(fp);
  node_t *program = parse(token);

  if (token->type != TOKEN_NUMBER) {
    error("expected TOKEN_NUMBER");
  }
  printf("  mov x0, %d\n", token->value.number);

  token = token->next;
  while (token->type == TOKEN_ADD) {
    token = token->next;
    if (token->type != TOKEN_NUMBER) {
      error("expected TOKEN_NUMBER");
    }
    printf("  add x0, x0, %d\n", token->value.number);
    token = token->next;
  }

  printf("  ret\n");

  return 0;
}
