#pragma once
#include <stdio.h>

typedef enum {
  TOKEN_EOF,
  TOKEN_ADD,
  TOKEN_SUB,
  TOKEN_NUMBER,
} tokentype_t;

typedef struct _token_t token_t;
struct _token_t {
  tokentype_t type;
  union {
    int number;
  } value;

  token_t *next;
};

token_t *new_token(tokentype_t type);

int read_char(FILE *fp);

token_t *read_number_token(FILE *fp);

token_t *read_next_token(FILE *fp);

token_t *tokenize(FILE *fp);
