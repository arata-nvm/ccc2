#include "parser.h"
#include "error.h"
#include <stdlib.h>

token_cursor_t *new_token_cursor(token_t *token) {
  token_cursor_t *cursor = calloc(1, sizeof(token_cursor_t));
  cursor->cur_token = token;
  return cursor;
}

token_t *peek(token_cursor_t *cursor) { return cursor->cur_token; }

token_t *consume(token_cursor_t *cursor) {
  token_t *cur_token = cursor->cur_token;
  if (cur_token->type != TOKEN_EOF) {
    cursor->cur_token = cur_token->next;
  }
  return cur_token;
}

token_t *expect(token_cursor_t *cursor, tokentype_t type) {
  token_t *cur_token = consume(cursor);
  if (cur_token->type != type) {
    error("unexpected token");
  }
  return cur_token;
}

node_t *new_node(nodetype_t type) {
  node_t *node = calloc(1, sizeof(node_t));
  node->type = type;
  return node;
}

node_t *parse_number(token_cursor_t *cursor) {
  int value = expect(cursor, TOKEN_NUMBER)->value.number;

  node_t *node = new_node(NODE_NUMBER);
  node->value.number = value;
  return node;
}

node_t *parse_add(token_cursor_t *cursor) {
  node_t *node = parse_number(cursor);

  while (consume(cursor)->type == TOKEN_ADD) {
    node_t *rhs = parse_number(cursor);

    node_t *node_new = new_node(NODE_ADD);
    node_new->value.binary.lhs = node;
    node_new->value.binary.rhs = rhs;
    node = node_new;
  }

  return node;
}

node_t *parse(token_t *token) {
  token_cursor_t *cursor = new_token_cursor(token);
  return parse_add(cursor);
}
