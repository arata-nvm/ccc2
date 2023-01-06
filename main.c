#include "codegen.h"
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

  token_t *token = tokenize(fp);
  stmt_t *program = parse(token);
  gen_code(program, stdout);

  return 0;
}
