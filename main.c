#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define error(msg)                                                             \
  {                                                                            \
    fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, msg);                    \
    exit(1);                                                                   \
  }

typedef enum {
  TOKEN_NUMBER,
} tokentype_t;

typedef struct {
  tokentype_t type;
  union {
    int number;
  } value;
} token_t;

token_t *read_next_token(FILE *fp) {
  char buf[256];
  int buf_index = 0;

  for (;;) {
    char c = fgetc(fp);

    if (isdigit(c)) {
      buf[buf_index] = c;
      buf_index += 1;
      continue;
    }

    if (isspace(c) && buf_index != 0) {
      buf[buf_index] = '\0';

      token_t *token = calloc(1, sizeof(token_t));
      token->type = TOKEN_NUMBER;
      token->value.number = atoi(buf);
      return token;
    }

    error("unexpected char");
  }
}

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
