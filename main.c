#include "codegen.h"
#include "error.h"
#include "parser.h"
#include "tokenizer.h"
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s <file>\n", argv[0]);
    return 1;
  }

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    panic("failed to open file '%s'\n", argv[1]);
  }

  token_t *token = tokenize(fp);
  global_stmt_t *program = parse(token);
  gen_code(program, stdout);

  return 0;
}
