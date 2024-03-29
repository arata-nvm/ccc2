#pragma once
#include <stdio.h>

typedef struct {
  int line;
  int column;
} pos_t;

char *pos_to_string(pos_t *pos);

typedef struct {
  FILE *fp;
  pos_t *cur_pos;
} tokenizer_ctx_t;

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
  TOKEN_INT,
  TOKEN_SIZEOF,
  TOKEN_BRACK_OPEN,
  TOKEN_BRACK_CLOSE,
  TOKEN_CHAR,
  TOKEN_STRING,
  TOKEN_AND,
  TOKEN_OR,
  TOKEN_NOT,
  TOKEN_XOR,
  TOKEN_NEG,
  TOKEN_SHL,
  TOKEN_SHR,
  TOKEN_ADDEQ,
  TOKEN_SUBEQ,
  TOKEN_MULEQ,
  TOKEN_DIVEQ,
  TOKEN_REMEQ,
  TOKEN_ANDEQ,
  TOKEN_OREQ,
  TOKEN_XOREQ,
  TOKEN_SHLEQ,
  TOKEN_SHREQ,
  TOKEN_LOGAND,
  TOKEN_LOGOR,
  TOKEN_BREAK,
  TOKEN_CONTINUE,
  TOKEN_SWITCH,
  TOKEN_CASE,
  TOKEN_COLON,
  TOKEN_DEFAULT,
  TOKEN_STRUCT,
  TOKEN_MEMBER,
  TOKEN_TYPEDEF,
  TOKEN_UNION,
  TOKEN_ENUM,
  TOKEN_VOID,
  TOKEN_VARARG,
  TOKEN_INC,
  TOKEN_DEC,
  TOKEN_ARROW,
  TOKEN_CHAR_LIT,
  TOKEN_EXTERN,
} tokentype_t;

typedef struct _token_t token_t;
struct _token_t {
  tokentype_t type;
  pos_t *pos;
  union {
    int number;
    char *ident;
    char *string;
    char char_;
  } value;

  token_t *next;
};

token_t *tokenize(FILE *fp);
