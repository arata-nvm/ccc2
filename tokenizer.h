#pragma once
#include <stdio.h>

typedef enum {
  TOKEN_NUMBER,
} tokentype_t;

typedef struct {
  tokentype_t type;
  union {
    int number;
  } value;
} token_t;

token_t *read_number_token(FILE *fp);

token_t *read_next_token(FILE *fp);
