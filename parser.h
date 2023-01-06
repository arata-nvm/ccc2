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
} nodetype_t;

typedef struct {
  nodetype_t type;
  union {
    int number;
    struct {
      struct node_t *lhs;
      struct node_t *rhs;
    } binary;
  } value;
} node_t;

node_t *new_node(nodetype_t type);

node_t *parse_number(token_cursor_t *cursor);

node_t *parse_add(token_cursor_t *cursor);

node_t *parse(token_t *token);
