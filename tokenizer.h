#pragma once
#include <stdio.h>

typedef enum {
  TOKEN_EOF,
  TOKEN_PLUS,
  TOKEN_NUMBER,
} tokentype_t;

typedef struct {
  tokentype_t type;
  union {
    int number;
  } value;

  struct token_t *next;
} token_t;

token_t *new_token(tokentype_t type);

int read_char(FILE *fp);

token_t *read_number_token(FILE *fp);

token_t *read_next_token(FILE *fp);

token_t *tokenize(FILE *fp);
