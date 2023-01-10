#include "parser.h"
#include "error.h"
#include "type.h"
#include <stdlib.h>

type_t *parse_type(token_cursor_t *cursor);
expr_t *parse_expr(token_cursor_t *cursor);
stmt_t *parse_stmt(token_cursor_t *cursor);

int is_unary_expr(exprtype_t type) {
  switch (type) {
  case EXPR_REF:
  case EXPR_DEREF:
  case EXPR_SIZEOF:
  case EXPR_NOT:
  case EXPR_NEG:
    return 1;
  default:
    return 0;
  }
}

int is_binary_expr(exprtype_t type) {
  switch (type) {
  case EXPR_ADD:
  case EXPR_SUB:
  case EXPR_MUL:
  case EXPR_DIV:
  case EXPR_REM:
  case EXPR_LT:
  case EXPR_LE:
  case EXPR_GT:
  case EXPR_GE:
  case EXPR_EQ:
  case EXPR_NE:
  case EXPR_AND:
  case EXPR_OR:
  case EXPR_XOR:
  case EXPR_SHL:
  case EXPR_SHR:
  case EXPR_LOGAND:
  case EXPR_LOGOR:
    return 1;
  default:
    return 0;
  }
}

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

token_t *consume_if(token_cursor_t *cursor, tokentype_t type) {
  if (peek(cursor)->type == type) {
    return consume(cursor);
  }

  return NULL;
}

token_t *expect(token_cursor_t *cursor, tokentype_t type) {
  token_t *cur_token = consume(cursor);
  if (cur_token->type != type) {
    panic("unexpected token: expected=%d, actual=%d\n", type, cur_token->type);
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

expr_t *new_string_expr(char *string) {
  expr_t *expr = new_expr(EXPR_STRING);
  expr->value.string = string;
  return expr;
}

expr_t *new_ident_expr(char *name) {
  expr_t *expr = new_expr(EXPR_IDENT);
  expr->value.ident = name;
  return expr;
}

expr_t *new_unary_expr(exprtype_t type, expr_t *expr2) {
  expr_t *expr = new_expr(type);
  expr->value.unary = expr2;
  return expr;
}

expr_t *new_binary_expr(exprtype_t type, expr_t *lhs, expr_t *rhs) {
  expr_t *expr = new_expr(type);
  expr->value.binary.lhs = lhs;
  expr->value.binary.rhs = rhs;
  return expr;
}

expr_t *new_assign_expr(exprtype_t type, expr_t *dst, expr_t *src) {
  expr_t *expr = new_expr(type);
  expr->value.assign.dst = dst;
  expr->value.assign.src = src;
  return expr;
}

argument_t *new_argument(expr_t *value) {
  argument_t *argument = calloc(1, sizeof(argument_t));
  argument->value = value;
  return argument;
}

stmt_t *new_stmt(stmttype_t type) {
  stmt_t *stmt = calloc(1, sizeof(stmt_t));
  stmt->type = type;
  return stmt;
}

stmt_list_t *new_stmt_list(stmt_t *stmt) {
  stmt_list_t *stmt_list = calloc(1, sizeof(stmt_list_t));
  stmt_list->stmt = stmt;
  return stmt_list;
}

stmt_case_t *new_stmt_case(int value, stmt_list_t *body) {
  stmt_case_t *stmt_case = calloc(1, sizeof(stmt_case_t));
  stmt_case->value = value;
  stmt_case->body = body;
  return stmt_case;
}

parameter_t *new_parameter(type_t *type, char *name) {
  parameter_t *parameter = calloc(1, sizeof(parameter_t));
  parameter->type = type;
  parameter->name = name;
  return parameter;
}

global_stmt_t *new_global_stmt(global_stmttype_t type) {
  global_stmt_t *gstmt = calloc(1, sizeof(global_stmt_t));
  gstmt->type = type;
  return gstmt;
}

program_t *new_program(global_stmt_t *body) {
  program_t *program = calloc(1, sizeof(program_t));
  program->body = body;
  return program;
}

argument_t *parse_arguments(token_cursor_t *cursor) {
  if (peek(cursor)->type == TOKEN_PAREN_CLOSE) {
    return NULL;
  }

  argument_t *head = new_argument(parse_expr(cursor));
  argument_t *cur = head;

  while (peek(cursor)->type == TOKEN_COMMA) {
    expect(cursor, TOKEN_COMMA);
    cur->next = new_argument(parse_expr(cursor));
    cur = cur->next;
  }

  return head;
}

expr_t *parse_primary(token_cursor_t *cursor) {
  switch (peek(cursor)->type) {
  case TOKEN_PAREN_OPEN:
    consume(cursor);
    expr_t *expr = parse_expr(cursor);
    expect(cursor, TOKEN_PAREN_CLOSE);
    return expr;
  case TOKEN_IDENT: {
    char *name = consume(cursor)->value.ident;
    return new_ident_expr(name);
  }
  case TOKEN_NUMBER:
    return new_number_expr(consume(cursor)->value.number);
  case TOKEN_STRING:
    return new_string_expr(consume(cursor)->value.string);
  default:
    panic("unexpected token: token=%d\n", peek(cursor)->type);
  }
}

expr_t *parse_postfix(token_cursor_t *cursor) {
  expr_t *expr = parse_primary(cursor);

  switch (peek(cursor)->type) {
  case TOKEN_PAREN_OPEN:
    consume(cursor);
    if (expr->type != EXPR_IDENT) {
      panic("not supported calling: expr=%d\n", expr->type);
    }
    expr_t *expr2 = new_expr(EXPR_CALL);
    expr2->value.call.name = expr->value.ident;
    expr2->value.call.args = parse_arguments(cursor);
    expect(cursor, TOKEN_PAREN_CLOSE);
    return expr2;
  case TOKEN_BRACK_OPEN:
    consume(cursor);
    expr_t *index = parse_expr(cursor);
    expect(cursor, TOKEN_BRACK_CLOSE);
    return new_unary_expr(EXPR_DEREF, new_binary_expr(EXPR_ADD, expr, index));
  case TOKEN_MEMBER:
    consume(cursor);
    char *name = expect(cursor, TOKEN_IDENT)->value.ident;
    expr_t *expr3 = new_expr(EXPR_MEMBER);
    expr3->value.member.expr = expr;
    expr3->value.member.name = name;
    return expr3;
  default:
    return expr;
  }
}

expr_t *parse_unary(token_cursor_t *cursor) {
  switch (peek(cursor)->type) {
  case TOKEN_ADD:
    consume(cursor);
    return parse_postfix(cursor);
  case TOKEN_SUB:
    consume(cursor);
    return new_binary_expr(EXPR_SUB, new_number_expr(0), parse_postfix(cursor));
  case TOKEN_AND:
    consume(cursor);
    return new_unary_expr(EXPR_REF, parse_postfix(cursor));
  case TOKEN_MUL:
    consume(cursor);
    return new_unary_expr(EXPR_DEREF, parse_unary(cursor));
  case TOKEN_NOT:
    consume(cursor);
    return new_unary_expr(EXPR_NOT, parse_unary(cursor));
  case TOKEN_NEG:
    consume(cursor);
    return new_unary_expr(EXPR_NEG, parse_unary(cursor));
  case TOKEN_SIZEOF:
    consume(cursor);
    return new_unary_expr(EXPR_SIZEOF, parse_unary(cursor));
  default:
    return parse_postfix(cursor);
  }
}

expr_t *parse_mul_div(token_cursor_t *cursor) {
  expr_t *expr = parse_unary(cursor);

  while (1) {
    switch (peek(cursor)->type) {
    case TOKEN_MUL:
      consume(cursor);
      expr = new_binary_expr(EXPR_MUL, expr, parse_unary(cursor));
      break;
    case TOKEN_DIV:
      consume(cursor);
      expr = new_binary_expr(EXPR_DIV, expr, parse_unary(cursor));
      break;
    case TOKEN_REM:
      consume(cursor);
      expr = new_binary_expr(EXPR_REM, expr, parse_unary(cursor));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_add_sub(token_cursor_t *cursor) {
  expr_t *expr = parse_mul_div(cursor);

  while (1) {
    switch (peek(cursor)->type) {
    case TOKEN_ADD:
      consume(cursor);
      expr = new_binary_expr(EXPR_ADD, expr, parse_mul_div(cursor));
      break;
    case TOKEN_SUB:
      consume(cursor);
      expr = new_binary_expr(EXPR_SUB, expr, parse_mul_div(cursor));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_shift(token_cursor_t *cursor) {
  expr_t *expr = parse_add_sub(cursor);

  while (1) {
    switch (peek(cursor)->type) {
    case TOKEN_SHL:
      consume(cursor);
      expr = new_binary_expr(EXPR_SHL, expr, parse_add_sub(cursor));
      break;
    case TOKEN_SHR:
      consume(cursor);
      expr = new_binary_expr(EXPR_SHR, expr, parse_add_sub(cursor));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_relational(token_cursor_t *cursor) {
  expr_t *expr = parse_shift(cursor);

  while (1) {
    switch (peek(cursor)->type) {
    case TOKEN_LT:
      consume(cursor);
      expr = new_binary_expr(EXPR_LT, expr, parse_shift(cursor));
      break;
    case TOKEN_LE:
      consume(cursor);
      expr = new_binary_expr(EXPR_LE, expr, parse_shift(cursor));
      break;
    case TOKEN_GT:
      consume(cursor);
      expr = new_binary_expr(EXPR_GT, expr, parse_shift(cursor));
      break;
    case TOKEN_GE:
      consume(cursor);
      expr = new_binary_expr(EXPR_GE, expr, parse_shift(cursor));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_equality(token_cursor_t *cursor) {
  expr_t *expr = parse_relational(cursor);

  while (1) {
    switch (peek(cursor)->type) {
    case TOKEN_EQ:
      consume(cursor);
      expr = new_binary_expr(EXPR_EQ, expr, parse_relational(cursor));
      break;
    case TOKEN_NE:
      consume(cursor);
      expr = new_binary_expr(EXPR_NE, expr, parse_relational(cursor));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_and(token_cursor_t *cursor) {
  expr_t *expr = parse_equality(cursor);

  while (consume_if(cursor, TOKEN_AND)) {
    expr = new_binary_expr(EXPR_AND, expr, parse_equality(cursor));
  }

  return expr;
}

expr_t *parse_xor(token_cursor_t *cursor) {
  expr_t *expr = parse_and(cursor);

  while (consume_if(cursor, TOKEN_XOR)) {
    expr = new_binary_expr(EXPR_XOR, expr, parse_and(cursor));
  }

  return expr;
}

expr_t *parse_or(token_cursor_t *cursor) {
  expr_t *expr = parse_xor(cursor);

  while (consume_if(cursor, TOKEN_OR)) {
    expr = new_binary_expr(EXPR_OR, expr, parse_xor(cursor));
  }

  return expr;
}

expr_t *parse_logand(token_cursor_t *cursor) {
  expr_t *expr = parse_or(cursor);

  while (consume_if(cursor, TOKEN_LOGAND)) {
    expr = new_binary_expr(EXPR_LOGAND, expr, parse_or(cursor));
  }

  return expr;
}

expr_t *parse_logor(token_cursor_t *cursor) {
  expr_t *expr = parse_logand(cursor);

  while (consume_if(cursor, TOKEN_LOGOR)) {
    expr = new_binary_expr(EXPR_LOGOR, expr, parse_logand(cursor));
  }

  return expr;
}

expr_t *parse_assign(token_cursor_t *cursor) {
  expr_t *expr = parse_logor(cursor);

  switch (peek(cursor)->type) {
  case TOKEN_ASSIGN:
    consume(cursor);
    return new_assign_expr(EXPR_ASSIGN, expr, parse_logor(cursor));
  case TOKEN_ADDEQ:
    consume(cursor);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_ADD, expr, parse_logor(cursor)));
  case TOKEN_SUBEQ:
    consume(cursor);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_SUB, expr, parse_logor(cursor)));
  case TOKEN_MULEQ:
    consume(cursor);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_MUL, expr, parse_logor(cursor)));
  case TOKEN_DIVEQ:
    consume(cursor);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_DIV, expr, parse_logor(cursor)));
  case TOKEN_REMEQ:
    consume(cursor);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_REM, expr, parse_logor(cursor)));
  case TOKEN_ANDEQ:
    consume(cursor);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_AND, expr, parse_logor(cursor)));
  case TOKEN_OREQ:
    consume(cursor);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_OR, expr, parse_logor(cursor)));
  case TOKEN_XOREQ:
    consume(cursor);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_XOR, expr, parse_logor(cursor)));
  case TOKEN_SHLEQ:
    consume(cursor);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_SHL, expr, parse_logor(cursor)));
  case TOKEN_SHREQ:
    consume(cursor);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_SHR, expr, parse_logor(cursor)));
  default:
    return expr;
  }
}

expr_t *parse_expr(token_cursor_t *cursor) { return parse_assign(cursor); }

stmt_t *parse_return(token_cursor_t *cursor) {
  stmt_t *stmt = new_stmt(STMT_RETURN);
  expect(cursor, TOKEN_RETURN);
  stmt->value.ret = parse_expr(cursor);
  expect(cursor, TOKEN_SEMICOLON);
  return stmt;
}

stmt_t *parse_if(token_cursor_t *cursor) {
  stmt_t *stmt = new_stmt(STMT_IF);
  expect(cursor, TOKEN_IF);
  expect(cursor, TOKEN_PAREN_OPEN);
  stmt->value.if_.cond = parse_expr(cursor);
  expect(cursor, TOKEN_PAREN_CLOSE);
  stmt->value.if_.then_ = parse_stmt(cursor);
  if (consume_if(cursor, TOKEN_ELSE)) {
    stmt->value.if_.else_ = parse_stmt(cursor);
  } else {
    stmt->value.if_.else_ = NULL;
  }
  return stmt;
}

stmt_t *parse_while(token_cursor_t *cursor) {
  stmt_t *stmt = new_stmt(STMT_WHILE);
  expect(cursor, TOKEN_WHILE);
  expect(cursor, TOKEN_PAREN_OPEN);
  stmt->value.while_.cond = parse_expr(cursor);
  expect(cursor, TOKEN_PAREN_CLOSE);
  stmt->value.while_.body = parse_stmt(cursor);
  return stmt;
}

stmt_t *parse_for(token_cursor_t *cursor) {
  stmt_t *stmt = new_stmt(STMT_FOR);
  expect(cursor, TOKEN_FOR);
  expect(cursor, TOKEN_PAREN_OPEN);

  // TODO
  if (peek(cursor)->type != TOKEN_SEMICOLON) {
    stmt->value.for_.init = parse_stmt(cursor);
  } else {
    stmt->value.for_.init = NULL;
  }

  if (peek(cursor)->type != TOKEN_SEMICOLON) {
    stmt->value.for_.cond = parse_expr(cursor);
  } else {
    stmt->value.for_.cond = NULL;
  }
  expect(cursor, TOKEN_SEMICOLON);

  if (peek(cursor)->type != TOKEN_PAREN_CLOSE) {
    stmt->value.for_.loop = parse_expr(cursor);
  } else {
    stmt->value.for_.loop = NULL;
  }
  expect(cursor, TOKEN_PAREN_CLOSE);

  stmt->value.for_.body = parse_stmt(cursor);

  return stmt;
}

stmt_t *parse_block(token_cursor_t *cursor) {
  expect(cursor, TOKEN_BRACE_OPEN);

  if (consume_if(cursor, TOKEN_BRACE_CLOSE)) {
    stmt_t *stmt = new_stmt(STMT_BLOCK);
    stmt->value.block = NULL;
    return stmt;
  }

  stmt_list_t *head = new_stmt_list(parse_stmt(cursor));
  stmt_list_t *cur = head;
  while (peek(cursor)->type != TOKEN_BRACE_CLOSE) {
    cur->next = new_stmt_list(parse_stmt(cursor));
    cur = cur->next;
  }
  expect(cursor, TOKEN_BRACE_CLOSE);

  stmt_t *stmt = new_stmt(STMT_BLOCK);
  stmt->value.block = head;
  return stmt;
}

type_t *parse_struct(token_cursor_t *cursor) {
  expect(cursor, TOKEN_STRUCT);

  char *tag = NULL;
  if (peek(cursor)->type == TOKEN_IDENT) {
    tag = consume(cursor)->value.ident;
  }

  if (!consume_if(cursor, TOKEN_BRACE_OPEN)) {
    return struct_of(tag, NULL);
  }

  type_t *type = parse_type(cursor);
  char *name = expect(cursor, TOKEN_IDENT)->value.ident;
  expect(cursor, TOKEN_SEMICOLON);

  struct_member_t *head = new_struct_member(type, name);
  struct_member_t *cur = head;

  while (peek(cursor)->type != TOKEN_BRACE_CLOSE) {
    type_t *type = parse_type(cursor);
    char *name = expect(cursor, TOKEN_IDENT)->value.ident;
    expect(cursor, TOKEN_SEMICOLON);
    cur->next = new_struct_member(type, name);
    cur = cur->next;
  }

  expect(cursor, TOKEN_BRACE_CLOSE);

  return struct_of(tag, head);
}

type_t *parse_type(token_cursor_t *cursor) {
  type_t *type;
  switch (peek(cursor)->type) {
  case TOKEN_CHAR:
    consume(cursor);
    type = new_type(TYPE_CHAR);
    break;
  case TOKEN_INT:
    consume(cursor);
    type = new_type(TYPE_INT);
    break;
  case TOKEN_STRUCT:
    type = parse_struct(cursor);
    break;
  default:
    panic("unknown type: token=%d\n", peek(cursor)->type);
  }

  while (consume_if(cursor, TOKEN_MUL)) {
    type = ptr_to(type);
  }

  return type;
}

type_t *parse_type_post(token_cursor_t *cursor, type_t *base_type) {
  if (!consume_if(cursor, TOKEN_BRACK_OPEN)) {
    return base_type;
  }

  int len = expect(cursor, TOKEN_NUMBER)->value.number;
  expect(cursor, TOKEN_BRACK_CLOSE);

  return array_of(base_type, len);
}

stmt_t *parse_define(token_cursor_t *cursor) {
  type_t *type = parse_type(cursor);
  char *name = expect(cursor, TOKEN_IDENT)->value.ident;
  type = parse_type_post(cursor, type);

  expr_t *value = NULL;
  if (consume_if(cursor, TOKEN_ASSIGN)) {
    value = parse_expr(cursor);
  }
  expect(cursor, TOKEN_SEMICOLON);

  stmt_t *stmt = new_stmt(STMT_DEFINE);
  stmt->value.define.type = type;
  stmt->value.define.name = name;
  stmt->value.define.value = value;
  return stmt;
}

stmt_case_t *parse_case(token_cursor_t *cursor) {
  int value = 0;
  if (!consume_if(cursor, TOKEN_DEFAULT)) {
    expect(cursor, TOKEN_CASE);
    value = expect(cursor, TOKEN_NUMBER)->value.number;
  }
  expect(cursor, TOKEN_COLON);

  stmt_list_t *head = new_stmt_list(parse_stmt(cursor));
  stmt_list_t *cur = head;
  while (peek(cursor)->type != TOKEN_CASE &&
         peek(cursor)->type != TOKEN_DEFAULT &&
         peek(cursor)->type != TOKEN_BRACE_CLOSE) {
    cur->next = new_stmt_list(parse_stmt(cursor));
    cur = cur->next;
  }

  return new_stmt_case(value, head);
}

stmt_t *parse_switch(token_cursor_t *cursor) {
  expect(cursor, TOKEN_SWITCH);
  expect(cursor, TOKEN_PAREN_OPEN);

  expr_t *value = parse_expr(cursor);

  expect(cursor, TOKEN_PAREN_CLOSE);
  expect(cursor, TOKEN_BRACE_OPEN);

  stmt_case_t *head = parse_case(cursor);
  stmt_case_t *cur = head;
  stmt_case_t *default_case = NULL;

  while (peek(cursor)->type == TOKEN_CASE ||
         peek(cursor)->type == TOKEN_DEFAULT) {
    if (peek(cursor)->type == TOKEN_DEFAULT) {
      default_case = parse_case(cursor);
      continue;
    }

    cur->next = parse_case(cursor);
    cur = cur->next;
  }

  expect(cursor, TOKEN_BRACE_CLOSE);

  stmt_t *stmt = new_stmt(STMT_SWITCH);
  stmt->value.switch_.value = value;
  stmt->value.switch_.cases = head;
  stmt->value.switch_.default_case = default_case;
  return stmt;
}

int is_type(token_t *token) {
  return token->type == TOKEN_CHAR || token->type == TOKEN_INT ||
         token->type == TOKEN_STRUCT;
}

stmt_t *parse_stmt(token_cursor_t *cursor) {
  switch (peek(cursor)->type) {
  case TOKEN_RETURN:
    return parse_return(cursor);
  case TOKEN_IF:
    return parse_if(cursor);
  case TOKEN_WHILE:
    return parse_while(cursor);
  case TOKEN_FOR:
    return parse_for(cursor);
  case TOKEN_BRACE_OPEN:
    return parse_block(cursor);
  case TOKEN_BREAK:
    consume(cursor);
    expect(cursor, TOKEN_SEMICOLON);
    return new_stmt(STMT_BREAK);
  case TOKEN_CONTINUE:
    consume(cursor);
    expect(cursor, TOKEN_SEMICOLON);
    return new_stmt(STMT_CONTINUE);
  case TOKEN_SWITCH:
    return parse_switch(cursor);
  default:
    if (is_type(peek(cursor))) {
      return parse_define(cursor);
    } else {
      stmt_t *stmt = new_stmt(STMT_EXPR);
      stmt->value.expr = parse_expr(cursor);
      expect(cursor, TOKEN_SEMICOLON);
      return stmt;
    }
  }
}

parameter_t *parse_parameter(token_cursor_t *cursor) {
  if (peek(cursor)->type == TOKEN_PAREN_CLOSE) {
    return NULL;
  }

  type_t *type = parse_type(cursor);
  char *name = expect(cursor, TOKEN_IDENT)->value.ident;
  parameter_t *head = new_parameter(type, name);
  parameter_t *cur = head;

  while (peek(cursor)->type == TOKEN_COMMA) {
    expect(cursor, TOKEN_COMMA);
    type = parse_type(cursor);
    name = expect(cursor, TOKEN_IDENT)->value.ident;
    cur->next = new_parameter(type, name);
    cur = cur->next;
  }

  return head;
}

global_stmt_t *parse_global_stmt(token_cursor_t *cursor) {
  global_stmt_t *gstmt;

  type_t *type = parse_type(cursor);
  if (type->kind == TYPE_STRUCT && peek(cursor)->type != TOKEN_IDENT) {
    expect(cursor, TOKEN_SEMICOLON);
    gstmt = new_global_stmt(GSTMT_STRUCT);
    gstmt->value.struct_ = type;
    return gstmt;
  }

  gstmt = new_global_stmt(GSTMT_FUNC);
  gstmt->value.func.ret_type = type;
  gstmt->value.func.name = expect(cursor, TOKEN_IDENT)->value.ident;
  expect(cursor, TOKEN_PAREN_OPEN);
  gstmt->value.func.params = parse_parameter(cursor);
  expect(cursor, TOKEN_PAREN_CLOSE);

  if (consume_if(cursor, TOKEN_SEMICOLON)) {
    gstmt->type = GSTMT_FUNC_DECL;
    return gstmt;
  }

  gstmt->value.func.body = parse_stmt(cursor);

  return gstmt;
}

program_t *parse(token_t *token) {
  token_cursor_t *cursor = new_token_cursor(token);

  global_stmt_t *head = parse_global_stmt(cursor);
  global_stmt_t *cur = head;
  while (peek(cursor)->type != TOKEN_EOF) {
    cur->next = parse_global_stmt(cursor);
    cur = cur->next;
  }

  return new_program(head);
}
