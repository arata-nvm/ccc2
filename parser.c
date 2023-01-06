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

expr_t *new_expr(exprtype_t type) {
  expr_t *expr = calloc(1, sizeof(expr_t));
  expr->type = type;
  return expr;
}

expr_t *new_number_expr(int value) {
  expr_t *expr = new_expr(EXPR_NUMBER);
  expr->value.number = value;
  return expr;
}

expr_t *new_ident_expr(char *name) {
  expr_t *expr = new_expr(EXPR_IDENT);
  expr->value.ident = name;
  return expr;
}

expr_t *new_binary_expr(exprtype_t type, expr_t *lhs, expr_t *rhs) {
  expr_t *expr = new_expr(type);
  expr->value.binary.lhs = lhs;
  expr->value.binary.rhs = rhs;
  return expr;
}

stmt_t *new_stmt(stmttype_t type) {
  stmt_t *stmt = calloc(1, sizeof(stmt_t));
  stmt->type = type;
  return stmt;
}

expr_t *parse_number(token_cursor_t *cursor) {
  int value = expect(cursor, TOKEN_NUMBER)->value.number;
  return new_number_expr(value);
}

expr_t *parse_primary(token_cursor_t *cursor) {
  if (peek(cursor)->type == TOKEN_PAREN_OPEN) {
    consume(cursor);
    expr_t *expr = parse_expr(cursor);
    expect(cursor, TOKEN_PAREN_CLOSE);
    return expr;
  } else if (peek(cursor)->type == TOKEN_IDENT) {
    char *name = consume(cursor)->value.ident;
    return new_ident_expr(name);
  }

  return parse_number(cursor);
}

expr_t *parse_unary(token_cursor_t *cursor) {
  if (peek(cursor)->type == TOKEN_ADD) {
    consume(cursor);
    return parse_primary(cursor);
  } else if (peek(cursor)->type == TOKEN_SUB) {
    consume(cursor);
    return new_binary_expr(EXPR_SUB, new_number_expr(0), parse_primary(cursor));
  }

  return parse_primary(cursor);
}

expr_t *parse_mul_div(token_cursor_t *cursor) {
  expr_t *expr = parse_unary(cursor);

  while (1) {
    if (peek(cursor)->type == TOKEN_MUL) {
      consume(cursor);
      expr = new_binary_expr(EXPR_MUL, expr, parse_unary(cursor));
    } else if (peek(cursor)->type == TOKEN_DIV) {
      consume(cursor);
      expr = new_binary_expr(EXPR_DIV, expr, parse_unary(cursor));
    } else if (peek(cursor)->type == TOKEN_REM) {
      consume(cursor);
      expr = new_binary_expr(EXPR_REM, expr, parse_unary(cursor));
    } else {
      break;
    }
  }

  return expr;
}

expr_t *parse_add_sub(token_cursor_t *cursor) {
  expr_t *expr = parse_mul_div(cursor);

  while (1) {
    if (peek(cursor)->type == TOKEN_ADD) {
      consume(cursor);
      expr = new_binary_expr(EXPR_ADD, expr, parse_mul_div(cursor));
    } else if (peek(cursor)->type == TOKEN_SUB) {
      consume(cursor);
      expr = new_binary_expr(EXPR_SUB, expr, parse_mul_div(cursor));
    } else {
      break;
    }
  }

  return expr;
}

expr_t *parse_relational(token_cursor_t *cursor) {
  expr_t *expr = parse_add_sub(cursor);

  while (1) {
    if (peek(cursor)->type == TOKEN_LT) {
      consume(cursor);
      expr = new_binary_expr(EXPR_LT, expr, parse_add_sub(cursor));
    } else if (peek(cursor)->type == TOKEN_LE) {
      consume(cursor);
      expr = new_binary_expr(EXPR_LE, expr, parse_add_sub(cursor));
    } else if (peek(cursor)->type == TOKEN_GT) {
      consume(cursor);
      expr = new_binary_expr(EXPR_GT, expr, parse_add_sub(cursor));
    } else if (peek(cursor)->type == TOKEN_GE) {
      consume(cursor);
      expr = new_binary_expr(EXPR_GE, expr, parse_add_sub(cursor));
    } else {
      break;
    }
  }

  return expr;
}

expr_t *parse_equality(token_cursor_t *cursor) {
  expr_t *expr = parse_relational(cursor);

  while (1) {
    if (peek(cursor)->type == TOKEN_EQ) {
      consume(cursor);
      expr = new_binary_expr(EXPR_EQ, expr, parse_relational(cursor));
    } else if (peek(cursor)->type == TOKEN_NE) {
      consume(cursor);
      expr = new_binary_expr(EXPR_NE, expr, parse_relational(cursor));
    } else {
      break;
    }
  }

  return expr;
}

expr_t *parse_assign(token_cursor_t *cursor) {
  expr_t *expr = parse_equality(cursor);

  if (peek(cursor)->type == TOKEN_ASSIGN) {
    consume(cursor);
    expr_t *expr2 = new_expr(EXPR_ASSIGN);
    expr2->value.assign.dst = expr;
    expr2->value.assign.src = parse_equality(cursor);
    return expr2;
  }

  return expr;
}

expr_t *parse_expr(token_cursor_t *cursor) { return parse_assign(cursor); }

stmt_t *parse_return(token_cursor_t *cursor) {
  expect(cursor, TOKEN_RETURN);
  stmt_t *stmt = new_stmt(STMT_RETURN);
  stmt->value.ret = parse_expr(cursor);
  return stmt;
}

stmt_t *parse_stmt(token_cursor_t *cursor) {
  stmt_t *stmt;
  if (peek(cursor)->type == TOKEN_RETURN) {
    stmt = parse_return(cursor);
  } else {
    stmt = new_stmt(STMT_EXPR);
    stmt->value.expr = parse_expr(cursor);
  }
  expect(cursor, TOKEN_SEMICOLON);
  return stmt;
}

stmt_t *parse(token_t *token) {
  token_cursor_t *cursor = new_token_cursor(token);
  stmt_t *head = parse_stmt(cursor);
  stmt_t *cur = head;

  while (peek(cursor)->type != TOKEN_EOF) {
    cur->next = parse_stmt(cursor);
    cur = cur->next;
  }

  return head;
}
