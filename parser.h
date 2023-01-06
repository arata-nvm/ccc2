#pragma once
#include "tokenizer.h"

typedef struct {
  token_t *cur_token;
} token_cursor_t;

token_cursor_t *new_token_cursor(token_t *token);

token_t *peek(token_cursor_t *cursor);

token_t *consume(token_cursor_t *cursor);

token_t *expect(token_cursor_t *cursor, tokentype_t type);

typedef enum {
  EXPR_NUMBER,
  EXPR_IDENT,
  EXPR_ADD,
  EXPR_SUB,
  EXPR_MUL,
  EXPR_DIV,
  EXPR_REM,
  EXPR_LT,
  EXPR_LE,
  EXPR_GT,
  EXPR_GE,
  EXPR_EQ,
  EXPR_NE,
  EXPR_ASSIGN,
} exprtype_t;

typedef struct _expr_t expr_t;
struct _expr_t {
  exprtype_t type;
  union {
    int number;
    char *ident;
    struct {
      expr_t *lhs;
      expr_t *rhs;
    } binary;
    struct {
      expr_t *dst;
      expr_t *src;
    } assign;
  } value;
};

expr_t *new_expr(exprtype_t type);

typedef enum {
  STMT_EXPR,
  STMT_RETURN,
} stmttype_t;

typedef struct _stmt_t stmt_t;
struct _stmt_t {
  stmttype_t type;
  union {
    expr_t *expr;
    expr_t *ret;
  } value;

  stmt_t *next;
};

expr_t *parse_number(token_cursor_t *cursor);

expr_t *parse_primary(token_cursor_t *cursor);

expr_t *parse_unary(token_cursor_t *cursor);

expr_t *parse_mul_div(token_cursor_t *cursor);

expr_t *parse_add_sub(token_cursor_t *cursor);

expr_t *parse_relational(token_cursor_t *cursor);

expr_t *parse_equality(token_cursor_t *cursor);

expr_t *parse_expr(token_cursor_t *token);

stmt_t *parse_return(token_cursor_t *cursor);

stmt_t *parse_stmt(token_cursor_t *token);

stmt_t *parse(token_t *token);
