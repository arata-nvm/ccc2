#include "parser.h"
#include "error.h"
#include "type.h"
#include <stdlib.h>
#include <string.h>

type_t *parse_type(parser_ctx_t *ctx);
expr_t *parse_expr(parser_ctx_t *ctx);
stmt_t *parse_stmt(parser_ctx_t *ctx);

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

parser_ctx_t *new_parser_ctx(token_t *token) {
  parser_ctx_t *ctx = calloc(1, sizeof(parser_ctx_t));
  ctx->cur_token = token;
  return ctx;
}

void add_typedef(parser_ctx_t *ctx, type_t *type, char *name) {
  typedef_t *typedef_ = calloc(1, sizeof(typedef_t));
  typedef_->type = type;
  typedef_->name = name;

  typedef_->next = ctx->typedefs;
  ctx->typedefs = typedef_;
}

typedef_t *find_typedef(parser_ctx_t *ctx, char *name) {
  typedef_t *cur = ctx->typedefs;
  while (cur) {
    if (!strcmp(cur->name, name)) {
      return cur;
    }
    cur = cur->next;
  }

  return NULL;
}

token_t *peek(parser_ctx_t *ctx) { return ctx->cur_token; }

token_t *consume(parser_ctx_t *ctx) {
  token_t *cur_token = ctx->cur_token;
  if (cur_token->type != TOKEN_EOF) {
    ctx->cur_token = cur_token->next;
  }
  return cur_token;
}

token_t *consume_if(parser_ctx_t *ctx, tokentype_t type) {
  if (peek(ctx)->type == type) {
    return consume(ctx);
  }

  return NULL;
}

token_t *expect(parser_ctx_t *ctx, tokentype_t type) {
  token_t *cur_token = consume(ctx);
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

argument_t *parse_arguments(parser_ctx_t *ctx) {
  if (peek(ctx)->type == TOKEN_PAREN_CLOSE) {
    return NULL;
  }

  argument_t *head = new_argument(parse_expr(ctx));
  argument_t *cur = head;

  while (peek(ctx)->type == TOKEN_COMMA) {
    expect(ctx, TOKEN_COMMA);
    cur->next = new_argument(parse_expr(ctx));
    cur = cur->next;
  }

  return head;
}

expr_t *parse_primary(parser_ctx_t *ctx) {
  switch (peek(ctx)->type) {
  case TOKEN_PAREN_OPEN:
    consume(ctx);
    expr_t *expr = parse_expr(ctx);
    expect(ctx, TOKEN_PAREN_CLOSE);
    return expr;
  case TOKEN_IDENT: {
    char *name = consume(ctx)->value.ident;
    return new_ident_expr(name);
  }
  case TOKEN_NUMBER:
    return new_number_expr(consume(ctx)->value.number);
  case TOKEN_STRING:
    return new_string_expr(consume(ctx)->value.string);
  default:
    panic("unexpected token: token=%d\n", peek(ctx)->type);
  }
}

expr_t *parse_postfix(parser_ctx_t *ctx) {
  expr_t *expr = parse_primary(ctx);

  switch (peek(ctx)->type) {
  case TOKEN_PAREN_OPEN:
    consume(ctx);
    if (expr->type != EXPR_IDENT) {
      panic("not supported calling: expr=%d\n", expr->type);
    }
    expr_t *expr2 = new_expr(EXPR_CALL);
    expr2->value.call.name = expr->value.ident;
    expr2->value.call.args = parse_arguments(ctx);
    expect(ctx, TOKEN_PAREN_CLOSE);
    return expr2;
  case TOKEN_BRACK_OPEN:
    consume(ctx);
    expr_t *index = parse_expr(ctx);
    expect(ctx, TOKEN_BRACK_CLOSE);
    return new_unary_expr(EXPR_DEREF, new_binary_expr(EXPR_ADD, expr, index));
  case TOKEN_MEMBER:
    consume(ctx);
    char *name = expect(ctx, TOKEN_IDENT)->value.ident;
    expr_t *expr3 = new_expr(EXPR_MEMBER);
    expr3->value.member.expr = expr;
    expr3->value.member.name = name;
    return expr3;
  default:
    return expr;
  }
}

expr_t *parse_unary(parser_ctx_t *ctx) {
  switch (peek(ctx)->type) {
  case TOKEN_ADD:
    consume(ctx);
    return parse_postfix(ctx);
  case TOKEN_SUB:
    consume(ctx);
    return new_binary_expr(EXPR_SUB, new_number_expr(0), parse_postfix(ctx));
  case TOKEN_AND:
    consume(ctx);
    return new_unary_expr(EXPR_REF, parse_postfix(ctx));
  case TOKEN_MUL:
    consume(ctx);
    return new_unary_expr(EXPR_DEREF, parse_unary(ctx));
  case TOKEN_NOT:
    consume(ctx);
    return new_unary_expr(EXPR_NOT, parse_unary(ctx));
  case TOKEN_NEG:
    consume(ctx);
    return new_unary_expr(EXPR_NEG, parse_unary(ctx));
  case TOKEN_SIZEOF:
    consume(ctx);
    return new_unary_expr(EXPR_SIZEOF, parse_unary(ctx));
  default:
    return parse_postfix(ctx);
  }
}

expr_t *parse_mul_div(parser_ctx_t *ctx) {
  expr_t *expr = parse_unary(ctx);

  while (1) {
    switch (peek(ctx)->type) {
    case TOKEN_MUL:
      consume(ctx);
      expr = new_binary_expr(EXPR_MUL, expr, parse_unary(ctx));
      break;
    case TOKEN_DIV:
      consume(ctx);
      expr = new_binary_expr(EXPR_DIV, expr, parse_unary(ctx));
      break;
    case TOKEN_REM:
      consume(ctx);
      expr = new_binary_expr(EXPR_REM, expr, parse_unary(ctx));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_add_sub(parser_ctx_t *ctx) {
  expr_t *expr = parse_mul_div(ctx);

  while (1) {
    switch (peek(ctx)->type) {
    case TOKEN_ADD:
      consume(ctx);
      expr = new_binary_expr(EXPR_ADD, expr, parse_mul_div(ctx));
      break;
    case TOKEN_SUB:
      consume(ctx);
      expr = new_binary_expr(EXPR_SUB, expr, parse_mul_div(ctx));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_shift(parser_ctx_t *ctx) {
  expr_t *expr = parse_add_sub(ctx);

  while (1) {
    switch (peek(ctx)->type) {
    case TOKEN_SHL:
      consume(ctx);
      expr = new_binary_expr(EXPR_SHL, expr, parse_add_sub(ctx));
      break;
    case TOKEN_SHR:
      consume(ctx);
      expr = new_binary_expr(EXPR_SHR, expr, parse_add_sub(ctx));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_relational(parser_ctx_t *ctx) {
  expr_t *expr = parse_shift(ctx);

  while (1) {
    switch (peek(ctx)->type) {
    case TOKEN_LT:
      consume(ctx);
      expr = new_binary_expr(EXPR_LT, expr, parse_shift(ctx));
      break;
    case TOKEN_LE:
      consume(ctx);
      expr = new_binary_expr(EXPR_LE, expr, parse_shift(ctx));
      break;
    case TOKEN_GT:
      consume(ctx);
      expr = new_binary_expr(EXPR_GT, expr, parse_shift(ctx));
      break;
    case TOKEN_GE:
      consume(ctx);
      expr = new_binary_expr(EXPR_GE, expr, parse_shift(ctx));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_equality(parser_ctx_t *ctx) {
  expr_t *expr = parse_relational(ctx);

  while (1) {
    switch (peek(ctx)->type) {
    case TOKEN_EQ:
      consume(ctx);
      expr = new_binary_expr(EXPR_EQ, expr, parse_relational(ctx));
      break;
    case TOKEN_NE:
      consume(ctx);
      expr = new_binary_expr(EXPR_NE, expr, parse_relational(ctx));
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_and(parser_ctx_t *ctx) {
  expr_t *expr = parse_equality(ctx);

  while (consume_if(ctx, TOKEN_AND)) {
    expr = new_binary_expr(EXPR_AND, expr, parse_equality(ctx));
  }

  return expr;
}

expr_t *parse_xor(parser_ctx_t *ctx) {
  expr_t *expr = parse_and(ctx);

  while (consume_if(ctx, TOKEN_XOR)) {
    expr = new_binary_expr(EXPR_XOR, expr, parse_and(ctx));
  }

  return expr;
}

expr_t *parse_or(parser_ctx_t *ctx) {
  expr_t *expr = parse_xor(ctx);

  while (consume_if(ctx, TOKEN_OR)) {
    expr = new_binary_expr(EXPR_OR, expr, parse_xor(ctx));
  }

  return expr;
}

expr_t *parse_logand(parser_ctx_t *ctx) {
  expr_t *expr = parse_or(ctx);

  while (consume_if(ctx, TOKEN_LOGAND)) {
    expr = new_binary_expr(EXPR_LOGAND, expr, parse_or(ctx));
  }

  return expr;
}

expr_t *parse_logor(parser_ctx_t *ctx) {
  expr_t *expr = parse_logand(ctx);

  while (consume_if(ctx, TOKEN_LOGOR)) {
    expr = new_binary_expr(EXPR_LOGOR, expr, parse_logand(ctx));
  }

  return expr;
}

expr_t *parse_assign(parser_ctx_t *ctx) {
  expr_t *expr = parse_logor(ctx);

  switch (peek(ctx)->type) {
  case TOKEN_ASSIGN:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr, parse_logor(ctx));
  case TOKEN_ADDEQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_ADD, expr, parse_logor(ctx)));
  case TOKEN_SUBEQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_SUB, expr, parse_logor(ctx)));
  case TOKEN_MULEQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_MUL, expr, parse_logor(ctx)));
  case TOKEN_DIVEQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_DIV, expr, parse_logor(ctx)));
  case TOKEN_REMEQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_REM, expr, parse_logor(ctx)));
  case TOKEN_ANDEQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_AND, expr, parse_logor(ctx)));
  case TOKEN_OREQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_OR, expr, parse_logor(ctx)));
  case TOKEN_XOREQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_XOR, expr, parse_logor(ctx)));
  case TOKEN_SHLEQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_SHL, expr, parse_logor(ctx)));
  case TOKEN_SHREQ:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr,
                           new_binary_expr(EXPR_SHR, expr, parse_logor(ctx)));
  default:
    return expr;
  }
}

expr_t *parse_expr(parser_ctx_t *ctx) { return parse_assign(ctx); }

stmt_t *parse_return(parser_ctx_t *ctx) {
  stmt_t *stmt = new_stmt(STMT_RETURN);
  expect(ctx, TOKEN_RETURN);
  stmt->value.ret = parse_expr(ctx);
  expect(ctx, TOKEN_SEMICOLON);
  return stmt;
}

stmt_t *parse_if(parser_ctx_t *ctx) {
  stmt_t *stmt = new_stmt(STMT_IF);
  expect(ctx, TOKEN_IF);
  expect(ctx, TOKEN_PAREN_OPEN);
  stmt->value.if_.cond = parse_expr(ctx);
  expect(ctx, TOKEN_PAREN_CLOSE);
  stmt->value.if_.then_ = parse_stmt(ctx);
  if (consume_if(ctx, TOKEN_ELSE)) {
    stmt->value.if_.else_ = parse_stmt(ctx);
  } else {
    stmt->value.if_.else_ = NULL;
  }
  return stmt;
}

stmt_t *parse_while(parser_ctx_t *ctx) {
  stmt_t *stmt = new_stmt(STMT_WHILE);
  expect(ctx, TOKEN_WHILE);
  expect(ctx, TOKEN_PAREN_OPEN);
  stmt->value.while_.cond = parse_expr(ctx);
  expect(ctx, TOKEN_PAREN_CLOSE);
  stmt->value.while_.body = parse_stmt(ctx);
  return stmt;
}

stmt_t *parse_for(parser_ctx_t *ctx) {
  stmt_t *stmt = new_stmt(STMT_FOR);
  expect(ctx, TOKEN_FOR);
  expect(ctx, TOKEN_PAREN_OPEN);

  // TODO
  if (peek(ctx)->type != TOKEN_SEMICOLON) {
    stmt->value.for_.init = parse_stmt(ctx);
  } else {
    stmt->value.for_.init = NULL;
  }

  if (peek(ctx)->type != TOKEN_SEMICOLON) {
    stmt->value.for_.cond = parse_expr(ctx);
  } else {
    stmt->value.for_.cond = NULL;
  }
  expect(ctx, TOKEN_SEMICOLON);

  if (peek(ctx)->type != TOKEN_PAREN_CLOSE) {
    stmt->value.for_.loop = parse_expr(ctx);
  } else {
    stmt->value.for_.loop = NULL;
  }
  expect(ctx, TOKEN_PAREN_CLOSE);

  stmt->value.for_.body = parse_stmt(ctx);

  return stmt;
}

stmt_t *parse_block(parser_ctx_t *ctx) {
  expect(ctx, TOKEN_BRACE_OPEN);

  if (consume_if(ctx, TOKEN_BRACE_CLOSE)) {
    stmt_t *stmt = new_stmt(STMT_BLOCK);
    stmt->value.block = NULL;
    return stmt;
  }

  stmt_list_t *head = new_stmt_list(parse_stmt(ctx));
  stmt_list_t *cur = head;
  while (peek(ctx)->type != TOKEN_BRACE_CLOSE) {
    cur->next = new_stmt_list(parse_stmt(ctx));
    cur = cur->next;
  }
  expect(ctx, TOKEN_BRACE_CLOSE);

  stmt_t *stmt = new_stmt(STMT_BLOCK);
  stmt->value.block = head;
  return stmt;
}

type_t *parse_struct(parser_ctx_t *ctx) {
  expect(ctx, TOKEN_STRUCT);

  char *tag = NULL;
  if (peek(ctx)->type == TOKEN_IDENT) {
    tag = consume(ctx)->value.ident;
  }

  if (!consume_if(ctx, TOKEN_BRACE_OPEN)) {
    return struct_of(tag, NULL);
  }

  type_t *type = parse_type(ctx);
  char *name = expect(ctx, TOKEN_IDENT)->value.ident;
  expect(ctx, TOKEN_SEMICOLON);

  struct_member_t *head = new_struct_member(type, name);
  struct_member_t *cur = head;

  while (peek(ctx)->type != TOKEN_BRACE_CLOSE) {
    type_t *type = parse_type(ctx);
    char *name = expect(ctx, TOKEN_IDENT)->value.ident;
    expect(ctx, TOKEN_SEMICOLON);
    cur->next = new_struct_member(type, name);
    cur = cur->next;
  }

  expect(ctx, TOKEN_BRACE_CLOSE);

  return struct_of(tag, head);
}

type_t *parse_type(parser_ctx_t *ctx) {
  type_t *type;
  switch (peek(ctx)->type) {
  case TOKEN_CHAR:
    consume(ctx);
    type = new_type(TYPE_CHAR);
    break;
  case TOKEN_INT:
    consume(ctx);
    type = new_type(TYPE_INT);
    break;
  case TOKEN_STRUCT:
    type = parse_struct(ctx);
    break;
  case TOKEN_IDENT: {
    char *type_name = consume(ctx)->value.ident;
    type = find_typedef(ctx, type_name)->type;
    break;
  }
  default:
    panic("unknown type: token=%d\n", peek(ctx)->type);
  }

  while (consume_if(ctx, TOKEN_MUL)) {
    type = ptr_to(type);
  }

  return type;
}

type_t *parse_type_post(parser_ctx_t *ctx, type_t *base_type) {
  if (!consume_if(ctx, TOKEN_BRACK_OPEN)) {
    return base_type;
  }

  int len = expect(ctx, TOKEN_NUMBER)->value.number;
  expect(ctx, TOKEN_BRACK_CLOSE);

  return array_of(base_type, len);
}

stmt_t *parse_define(parser_ctx_t *ctx) {
  type_t *type = parse_type(ctx);
  char *name = expect(ctx, TOKEN_IDENT)->value.ident;
  type = parse_type_post(ctx, type);

  expr_t *value = NULL;
  if (consume_if(ctx, TOKEN_ASSIGN)) {
    value = parse_expr(ctx);
  }
  expect(ctx, TOKEN_SEMICOLON);

  stmt_t *stmt = new_stmt(STMT_DEFINE);
  stmt->value.define.type = type;
  stmt->value.define.name = name;
  stmt->value.define.value = value;
  return stmt;
}

stmt_case_t *parse_case(parser_ctx_t *ctx) {
  int value = 0;
  if (!consume_if(ctx, TOKEN_DEFAULT)) {
    expect(ctx, TOKEN_CASE);
    value = expect(ctx, TOKEN_NUMBER)->value.number;
  }
  expect(ctx, TOKEN_COLON);

  stmt_list_t *head = new_stmt_list(parse_stmt(ctx));
  stmt_list_t *cur = head;
  while (peek(ctx)->type != TOKEN_CASE && peek(ctx)->type != TOKEN_DEFAULT &&
         peek(ctx)->type != TOKEN_BRACE_CLOSE) {
    cur->next = new_stmt_list(parse_stmt(ctx));
    cur = cur->next;
  }

  return new_stmt_case(value, head);
}

stmt_t *parse_switch(parser_ctx_t *ctx) {
  expect(ctx, TOKEN_SWITCH);
  expect(ctx, TOKEN_PAREN_OPEN);

  expr_t *value = parse_expr(ctx);

  expect(ctx, TOKEN_PAREN_CLOSE);
  expect(ctx, TOKEN_BRACE_OPEN);

  stmt_case_t *head = parse_case(ctx);
  stmt_case_t *cur = head;
  stmt_case_t *default_case = NULL;

  while (peek(ctx)->type == TOKEN_CASE || peek(ctx)->type == TOKEN_DEFAULT) {
    if (peek(ctx)->type == TOKEN_DEFAULT) {
      default_case = parse_case(ctx);
      continue;
    }

    cur->next = parse_case(ctx);
    cur = cur->next;
  }

  expect(ctx, TOKEN_BRACE_CLOSE);

  stmt_t *stmt = new_stmt(STMT_SWITCH);
  stmt->value.switch_.value = value;
  stmt->value.switch_.cases = head;
  stmt->value.switch_.default_case = default_case;
  return stmt;
}

int is_type(parser_ctx_t *ctx, token_t *token) {
  return token->type == TOKEN_CHAR || token->type == TOKEN_INT ||
         token->type == TOKEN_STRUCT ||
         (token->type == TOKEN_IDENT && find_typedef(ctx, token->value.ident));
}

stmt_t *parse_stmt(parser_ctx_t *ctx) {
  switch (peek(ctx)->type) {
  case TOKEN_RETURN:
    return parse_return(ctx);
  case TOKEN_IF:
    return parse_if(ctx);
  case TOKEN_WHILE:
    return parse_while(ctx);
  case TOKEN_FOR:
    return parse_for(ctx);
  case TOKEN_BRACE_OPEN:
    return parse_block(ctx);
  case TOKEN_BREAK:
    consume(ctx);
    expect(ctx, TOKEN_SEMICOLON);
    return new_stmt(STMT_BREAK);
  case TOKEN_CONTINUE:
    consume(ctx);
    expect(ctx, TOKEN_SEMICOLON);
    return new_stmt(STMT_CONTINUE);
  case TOKEN_SWITCH:
    return parse_switch(ctx);
  default:
    if (is_type(ctx, peek(ctx))) {
      return parse_define(ctx);
    } else {
      stmt_t *stmt = new_stmt(STMT_EXPR);
      stmt->value.expr = parse_expr(ctx);
      expect(ctx, TOKEN_SEMICOLON);
      return stmt;
    }
  }
}

parameter_t *parse_parameter(parser_ctx_t *ctx) {
  if (peek(ctx)->type == TOKEN_PAREN_CLOSE) {
    return NULL;
  }

  type_t *type = parse_type(ctx);
  char *name = expect(ctx, TOKEN_IDENT)->value.ident;
  parameter_t *head = new_parameter(type, name);
  parameter_t *cur = head;

  while (peek(ctx)->type == TOKEN_COMMA) {
    expect(ctx, TOKEN_COMMA);
    type = parse_type(ctx);
    name = expect(ctx, TOKEN_IDENT)->value.ident;
    cur->next = new_parameter(type, name);
    cur = cur->next;
  }

  return head;
}

global_stmt_t *parse_typedef(parser_ctx_t *ctx) {
  expect(ctx, TOKEN_TYPEDEF);

  type_t *type = parse_type(ctx);
  char *name = expect(ctx, TOKEN_IDENT)->value.ident;
  expect(ctx, TOKEN_SEMICOLON);

  add_typedef(ctx, type, name);

  return new_global_stmt(GSTMT_TYPEDEF);
}

global_stmt_t *parse_global_stmt(parser_ctx_t *ctx) {
  global_stmt_t *gstmt;

  if (peek(ctx)->type == TOKEN_TYPEDEF) {
    return parse_typedef(ctx);
  }

  type_t *type = parse_type(ctx);
  if (type->kind == TYPE_STRUCT && peek(ctx)->type != TOKEN_IDENT) {
    expect(ctx, TOKEN_SEMICOLON);
    gstmt = new_global_stmt(GSTMT_STRUCT);
    gstmt->value.struct_ = type;
    return gstmt;
  }

  gstmt = new_global_stmt(GSTMT_FUNC);
  gstmt->value.func.ret_type = type;
  gstmt->value.func.name = expect(ctx, TOKEN_IDENT)->value.ident;
  expect(ctx, TOKEN_PAREN_OPEN);
  gstmt->value.func.params = parse_parameter(ctx);
  expect(ctx, TOKEN_PAREN_CLOSE);

  if (consume_if(ctx, TOKEN_SEMICOLON)) {
    gstmt->type = GSTMT_FUNC_DECL;
    return gstmt;
  }

  gstmt->value.func.body = parse_stmt(ctx);

  return gstmt;
}

program_t *parse(token_t *token) {
  parser_ctx_t *ctx = new_parser_ctx(token);

  global_stmt_t *head = parse_global_stmt(ctx);
  global_stmt_t *cur = head;
  while (peek(ctx)->type != TOKEN_EOF) {
    cur->next = parse_global_stmt(ctx);
    if (cur->next) {
      cur = cur->next;
    }
  }

  return new_program(head);
}
