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
  NODE_NUMBER,
  NODE_ADD,
  NODE_SUB,
  NODE_MUL,
  NODE_DIV,
  NODE_REM,
  NODE_LT,
  NODE_LE,
  NODE_GT,
  NODE_GE,
  NODE_EQ,
  NODE_NE,
} nodetype_t;

typedef struct _node_t node_t;
struct _node_t {
  nodetype_t type;
  union {
    int number;
    struct {
      node_t *lhs;
      node_t *rhs;
    } binary;
  } value;
};

node_t *new_node(nodetype_t type);

node_t *parse_number(token_cursor_t *cursor);

node_t *parse_primary(token_cursor_t *cursor);

node_t *parse_unary(token_cursor_t *cursor);

node_t *parse_mul_div(token_cursor_t *cursor);

node_t *parse_add_sub(token_cursor_t *cursor);

node_t *parse_relational(token_cursor_t *cursor);

node_t *parse_equality(token_cursor_t *cursor);

node_t *parse(token_t *token);
