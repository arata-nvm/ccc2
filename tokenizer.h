#pragma once
#include <stdio.h>

typedef enum {
  TOKEN_EOF,
  TOKEN_NUMBER,
  TOKEN_IDENT,
  TOKEN_ADD,
  TOKEN_SUB,
  TOKEN_MUL,
  TOKEN_DIV,
  TOKEN_REM,
  TOKEN_PAREN_OPEN,
  TOKEN_PAREN_CLOSE,
  TOKEN_LT,
  TOKEN_LE,
  TOKEN_GT,
  TOKEN_GE,
  TOKEN_EQ,
  TOKEN_NE,
  TOKEN_SEMICOLON,
  TOKEN_ASSIGN,
  TOKEN_BRACE_OPEN,
  TOKEN_BRACE_CLOSE,
  TOKEN_RETURN,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_FOR,
  TOKEN_COMMA,
  TOKEN_REF,
  TOKEN_INT,
  TOKEN_SIZEOF,
  TOKEN_BRACK_OPEN,
  TOKEN_BRACK_CLOSE,
  TOKEN_CHAR,
} tokentype_t;

typedef struct _token_t token_t;
struct _token_t {
  tokentype_t type;
  union {
    int number;
    char *ident;
  } value;

  token_t *next;
};

token_t *tokenize(FILE *fp);
