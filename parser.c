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
  case EXPR_INC_PRE:
  case EXPR_INC_POST:
  case EXPR_DEC_PRE:
  case EXPR_DEC_POST:
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

int is_type(parser_ctx_t *ctx, token_t *token) {
  return token->type == TOKEN_CHAR || token->type == TOKEN_INT ||
         token->type == TOKEN_STRUCT || token->type == TOKEN_UNION ||
         token->type == TOKEN_ENUM || token->type == TOKEN_VOID ||
         (token->type == TOKEN_IDENT && find_typedef(ctx, token->value.ident));
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
    error(cur_token->pos, "unexpected token: expected=%d, actual=%d\n", type,
          cur_token->type);
  }
  return cur_token;
}

expr_t *new_expr(exprtype_t type, pos_t *pos) {
  expr_t *expr = calloc(1, sizeof(expr_t));
  expr->type = type;
  expr->pos = pos;
  return expr;
}

expr_t *new_char_expr(char value, pos_t *pos) {
  expr_t *expr = new_expr(EXPR_CHAR, pos);
  expr->value.char_ = value;
  return expr;
}

expr_t *new_number_expr(int value, pos_t *pos) {
  expr_t *expr = new_expr(EXPR_NUMBER, pos);
  expr->value.number = value;
  return expr;
}

expr_t *new_string_expr(char *string, pos_t *pos) {
  expr_t *expr = new_expr(EXPR_STRING, pos);
  expr->value.string = string;
  return expr;
}

expr_t *new_ident_expr(char *name, pos_t *pos) {
  expr_t *expr = new_expr(EXPR_IDENT, pos);
  expr->value.ident = name;
  return expr;
}

expr_t *new_unary_expr(exprtype_t type, expr_t *expr2, pos_t *pos) {
  expr_t *expr = new_expr(type, pos);
  expr->value.unary = expr2;
  return expr;
}

expr_t *new_binary_expr(exprtype_t type, expr_t *lhs, expr_t *rhs, pos_t *pos) {
  expr_t *expr = new_expr(type, pos);
  expr->value.binary.lhs = lhs;
  expr->value.binary.rhs = rhs;
  return expr;
}

expr_t *new_assign_expr(exprtype_t type, expr_t *dst, expr_t *src, pos_t *pos) {
  expr_t *expr = new_expr(type, pos);
  expr->value.assign.dst = dst;
  expr->value.assign.src = src;
  return expr;
}

argument_t *new_argument(expr_t *value) {
  argument_t *argument = calloc(1, sizeof(argument_t));
  argument->value = value;
  return argument;
}

stmt_t *new_stmt(stmttype_t type, pos_t *pos) {
  stmt_t *stmt = calloc(1, sizeof(stmt_t));
  stmt->type = type;
  stmt->pos = pos;
  return stmt;
}

stmt_list_t *new_stmt_list(stmt_t *stmt) {
  stmt_list_t *stmt_list = calloc(1, sizeof(stmt_list_t));
  stmt_list->stmt = stmt;
  return stmt_list;
}

stmt_case_t *new_stmt_case(expr_t *value, stmt_list_t *body) {
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

global_stmt_t *new_global_stmt(global_stmttype_t type, pos_t *pos) {
  global_stmt_t *gstmt = calloc(1, sizeof(global_stmt_t));
  gstmt->type = type;
  gstmt->pos = pos;
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
  pos_t *pos = peek(ctx)->pos;
  switch (peek(ctx)->type) {
  case TOKEN_PAREN_OPEN:
    consume(ctx);
    expr_t *expr = parse_expr(ctx);
    expect(ctx, TOKEN_PAREN_CLOSE);
    return expr;
  case TOKEN_IDENT: {
    char *name = consume(ctx)->value.ident;
    return new_ident_expr(name, pos);
  }
  case TOKEN_CHAR_LIT:
    return new_char_expr(consume(ctx)->value.char_, pos);
  case TOKEN_NUMBER:
    return new_number_expr(consume(ctx)->value.number, pos);
  case TOKEN_STRING:
    return new_string_expr(consume(ctx)->value.string, pos);
  default:
    error(pos, "unexpected token: token=%d\n", peek(ctx)->type);
  }
}

expr_t *parse_postfix(parser_ctx_t *ctx) {
  expr_t *expr = parse_primary(ctx);

  while (1) {
    pos_t *pos = peek(ctx)->pos;
    switch (peek(ctx)->type) {
    case TOKEN_PAREN_OPEN:
      consume(ctx);
      if (expr->type != EXPR_IDENT) {
        error(pos, "not supported calling: expr=%d\n", expr->type);
      }
      expr_t *expr2 = new_expr(EXPR_CALL, pos);
      expr2->value.call.name = expr->value.ident;
      expr2->value.call.args = parse_arguments(ctx);
      expect(ctx, TOKEN_PAREN_CLOSE);
      expr = expr2;
      break;
    case TOKEN_BRACK_OPEN:
      consume(ctx);
      expr_t *index = parse_expr(ctx);
      expect(ctx, TOKEN_BRACK_CLOSE);
      expr = new_unary_expr(EXPR_DEREF,
                            new_binary_expr(EXPR_ADD, expr, index, pos), pos);
      break;
    case TOKEN_MEMBER:
      consume(ctx);
      char *name = expect(ctx, TOKEN_IDENT)->value.ident;
      expr_t *expr3 = new_expr(EXPR_MEMBER, pos);
      expr3->value.member.expr = expr;
      expr3->value.member.name = name;
      expr = expr3;
      break;
    case TOKEN_INC:
      consume(ctx);
      return new_unary_expr(EXPR_INC_POST, expr, pos);
    case TOKEN_DEC:
      consume(ctx);
      return new_unary_expr(EXPR_DEC_POST, expr, pos);
    case TOKEN_ARROW:
      consume(ctx);
      char *name2 = expect(ctx, TOKEN_IDENT)->value.ident;
      expr_t *expr4 = new_expr(EXPR_MEMBER, pos);
      expr4->value.member.expr = new_unary_expr(EXPR_DEREF, expr, pos);
      expr4->value.member.name = name2;
      expr = expr4;
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_unary(parser_ctx_t *ctx) {
  pos_t *pos = peek(ctx)->pos;

  switch (peek(ctx)->type) {
  case TOKEN_ADD:
    consume(ctx);
    return parse_postfix(ctx);
  case TOKEN_SUB:
    consume(ctx);
    return new_binary_expr(EXPR_SUB, new_number_expr(0, pos),
                           parse_postfix(ctx), pos);
  case TOKEN_AND:
    consume(ctx);
    return new_unary_expr(EXPR_REF, parse_postfix(ctx), pos);
  case TOKEN_MUL:
    consume(ctx);
    return new_unary_expr(EXPR_DEREF, parse_unary(ctx), pos);
  case TOKEN_NOT:
    consume(ctx);
    return new_unary_expr(EXPR_NOT, parse_unary(ctx), pos);
  case TOKEN_NEG:
    consume(ctx);
    return new_unary_expr(EXPR_NEG, parse_unary(ctx), pos);
  case TOKEN_SIZEOF:
    consume(ctx);
    expect(ctx, TOKEN_PAREN_OPEN);
    expr_t *expr = new_expr(EXPR_SIZEOF, pos);
    if (is_type(ctx, peek(ctx))) {
      expr->value.sizeof_.type = parse_type(ctx);
    } else {
      expr->value.sizeof_.expr = parse_unary(ctx);
    }
    expect(ctx, TOKEN_PAREN_CLOSE);
    return expr;
  case TOKEN_INC:
    consume(ctx);
    return new_unary_expr(EXPR_INC_PRE, parse_postfix(ctx), pos);
  case TOKEN_DEC:
    consume(ctx);
    return new_unary_expr(EXPR_DEC_PRE, parse_postfix(ctx), pos);
  default:
    return parse_postfix(ctx);
  }
}

expr_t *parse_mul_div(parser_ctx_t *ctx) {
  expr_t *expr = parse_unary(ctx);

  while (1) {
    pos_t *pos = peek(ctx)->pos;
    switch (peek(ctx)->type) {
    case TOKEN_MUL:
      consume(ctx);
      expr = new_binary_expr(EXPR_MUL, expr, parse_unary(ctx), pos);
      break;
    case TOKEN_DIV:
      consume(ctx);
      expr = new_binary_expr(EXPR_DIV, expr, parse_unary(ctx), pos);
      break;
    case TOKEN_REM:
      consume(ctx);
      expr = new_binary_expr(EXPR_REM, expr, parse_unary(ctx), pos);
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_add_sub(parser_ctx_t *ctx) {
  expr_t *expr = parse_mul_div(ctx);

  while (1) {
    pos_t *pos = peek(ctx)->pos;
    switch (peek(ctx)->type) {
    case TOKEN_ADD:
      consume(ctx);
      expr = new_binary_expr(EXPR_ADD, expr, parse_mul_div(ctx), pos);
      break;
    case TOKEN_SUB:
      consume(ctx);
      expr = new_binary_expr(EXPR_SUB, expr, parse_mul_div(ctx), pos);
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_shift(parser_ctx_t *ctx) {
  expr_t *expr = parse_add_sub(ctx);

  while (1) {
    pos_t *pos = peek(ctx)->pos;
    switch (peek(ctx)->type) {
    case TOKEN_SHL:
      consume(ctx);
      expr = new_binary_expr(EXPR_SHL, expr, parse_add_sub(ctx), pos);
      break;
    case TOKEN_SHR:
      consume(ctx);
      expr = new_binary_expr(EXPR_SHR, expr, parse_add_sub(ctx), pos);
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_relational(parser_ctx_t *ctx) {
  expr_t *expr = parse_shift(ctx);

  while (1) {
    pos_t *pos = peek(ctx)->pos;
    switch (peek(ctx)->type) {
    case TOKEN_LT:
      consume(ctx);
      expr = new_binary_expr(EXPR_LT, expr, parse_shift(ctx), pos);
      break;
    case TOKEN_LE:
      consume(ctx);
      expr = new_binary_expr(EXPR_LE, expr, parse_shift(ctx), pos);
      break;
    case TOKEN_GT:
      consume(ctx);
      expr = new_binary_expr(EXPR_GT, expr, parse_shift(ctx), pos);
      break;
    case TOKEN_GE:
      consume(ctx);
      expr = new_binary_expr(EXPR_GE, expr, parse_shift(ctx), pos);
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_equality(parser_ctx_t *ctx) {
  expr_t *expr = parse_relational(ctx);

  while (1) {
    pos_t *pos = peek(ctx)->pos;
    switch (peek(ctx)->type) {
    case TOKEN_EQ:
      consume(ctx);
      expr = new_binary_expr(EXPR_EQ, expr, parse_relational(ctx), pos);
      break;
    case TOKEN_NE:
      consume(ctx);
      expr = new_binary_expr(EXPR_NE, expr, parse_relational(ctx), pos);
      break;
    default:
      return expr;
    }
  }
}

expr_t *parse_and(parser_ctx_t *ctx) {
  expr_t *expr = parse_equality(ctx);

  pos_t *pos = peek(ctx)->pos;
  while (consume_if(ctx, TOKEN_AND)) {
    expr = new_binary_expr(EXPR_AND, expr, parse_equality(ctx), pos);
    pos = peek(ctx)->pos;
  }

  return expr;
}

expr_t *parse_xor(parser_ctx_t *ctx) {
  expr_t *expr = parse_and(ctx);

  pos_t *pos = peek(ctx)->pos;
  while (consume_if(ctx, TOKEN_XOR)) {
    expr = new_binary_expr(EXPR_XOR, expr, parse_and(ctx), pos);
    pos = peek(ctx)->pos;
  }

  return expr;
}

expr_t *parse_or(parser_ctx_t *ctx) {
  expr_t *expr = parse_xor(ctx);

  pos_t *pos = peek(ctx)->pos;
  while (consume_if(ctx, TOKEN_OR)) {
    expr = new_binary_expr(EXPR_OR, expr, parse_xor(ctx), pos);
    pos = peek(ctx)->pos;
  }

  return expr;
}

expr_t *parse_logand(parser_ctx_t *ctx) {
  expr_t *expr = parse_or(ctx);

  pos_t *pos = peek(ctx)->pos;
  while (consume_if(ctx, TOKEN_LOGAND)) {
    expr = new_binary_expr(EXPR_LOGAND, expr, parse_or(ctx), pos);
    pos = peek(ctx)->pos;
  }

  return expr;
}

expr_t *parse_logor(parser_ctx_t *ctx) {
  expr_t *expr = parse_logand(ctx);

  pos_t *pos = peek(ctx)->pos;
  while (consume_if(ctx, TOKEN_LOGOR)) {
    expr = new_binary_expr(EXPR_LOGOR, expr, parse_logand(ctx), pos);
    pos = peek(ctx)->pos;
  }

  return expr;
}

expr_t *parse_assign(parser_ctx_t *ctx) {
  expr_t *expr = parse_logor(ctx);

  pos_t *pos = peek(ctx)->pos;
  switch (peek(ctx)->type) {
  case TOKEN_ASSIGN:
    consume(ctx);
    return new_assign_expr(EXPR_ASSIGN, expr, parse_logor(ctx), pos);
  case TOKEN_ADDEQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_ADD, expr, parse_logor(ctx), pos), pos);
  case TOKEN_SUBEQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_SUB, expr, parse_logor(ctx), pos), pos);
  case TOKEN_MULEQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_MUL, expr, parse_logor(ctx), pos), pos);
  case TOKEN_DIVEQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_DIV, expr, parse_logor(ctx), pos), pos);
  case TOKEN_REMEQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_REM, expr, parse_logor(ctx), pos), pos);
  case TOKEN_ANDEQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_AND, expr, parse_logor(ctx), pos), pos);
  case TOKEN_OREQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_OR, expr, parse_logor(ctx), pos), pos);
  case TOKEN_XOREQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_XOR, expr, parse_logor(ctx), pos), pos);
  case TOKEN_SHLEQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_SHL, expr, parse_logor(ctx), pos), pos);
  case TOKEN_SHREQ:
    consume(ctx);
    return new_assign_expr(
        EXPR_ASSIGN, expr,
        new_binary_expr(EXPR_SHR, expr, parse_logor(ctx), pos), pos);
  default:
    return expr;
  }
}

expr_t *parse_expr(parser_ctx_t *ctx) { return parse_assign(ctx); }

stmt_t *parse_return(parser_ctx_t *ctx) {
  pos_t *pos = expect(ctx, TOKEN_RETURN)->pos;
  stmt_t *stmt = new_stmt(STMT_RETURN, pos);
  if (consume_if(ctx, TOKEN_SEMICOLON)) {
    return stmt;
  }

  stmt->value.ret = parse_expr(ctx);
  expect(ctx, TOKEN_SEMICOLON);
  return stmt;
}

stmt_t *parse_if(parser_ctx_t *ctx) {
  pos_t *pos = expect(ctx, TOKEN_IF)->pos;
  stmt_t *stmt = new_stmt(STMT_IF, pos);

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
  pos_t *pos = expect(ctx, TOKEN_WHILE)->pos;
  stmt_t *stmt = new_stmt(STMT_WHILE, pos);

  expect(ctx, TOKEN_PAREN_OPEN);
  stmt->value.while_.cond = parse_expr(ctx);
  expect(ctx, TOKEN_PAREN_CLOSE);
  stmt->value.while_.body = parse_stmt(ctx);
  return stmt;
}

stmt_t *parse_for(parser_ctx_t *ctx) {
  pos_t *pos = expect(ctx, TOKEN_FOR)->pos;
  stmt_t *stmt = new_stmt(STMT_FOR, pos);

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
  pos_t *pos = expect(ctx, TOKEN_BRACE_OPEN)->pos;

  stmt_t *stmt;
  if (consume_if(ctx, TOKEN_BRACE_CLOSE)) {
    stmt = new_stmt(STMT_BLOCK, pos);
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

  stmt = new_stmt(STMT_BLOCK, pos);
  stmt->value.block = head;
  return stmt;
}

type_t *parse_struct_union(parser_ctx_t *ctx) {
  int is_union = 0;
  if (consume_if(ctx, TOKEN_UNION)) {
    is_union = 1;
  } else {
    expect(ctx, TOKEN_STRUCT);
  }

  char *tag = NULL;
  if (peek(ctx)->type == TOKEN_IDENT) {
    tag = consume(ctx)->value.ident;
  }

  if (!consume_if(ctx, TOKEN_BRACE_OPEN)) {
    if (is_union) {
      return union_of(tag, NULL);
    } else {
      return struct_of(tag, NULL);
    }
  }

  type_t *type = parse_type(ctx);
  char *name = expect(ctx, TOKEN_IDENT)->value.ident;
  expect(ctx, TOKEN_SEMICOLON);

  struct_member_t *head = new_struct_member(type, name);
  struct_member_t *cur = head;

  while (peek(ctx)->type != TOKEN_BRACE_CLOSE) {
    type = parse_type(ctx);
    name = expect(ctx, TOKEN_IDENT)->value.ident;
    expect(ctx, TOKEN_SEMICOLON);
    cur->next = new_struct_member(type, name);
    cur = cur->next;
  }

  expect(ctx, TOKEN_BRACE_CLOSE);

  if (is_union) {
    return union_of(tag, head);
  } else {
    return struct_of(tag, head);
  }
}

type_t *parse_enum(parser_ctx_t *ctx) {
  expect(ctx, TOKEN_ENUM);

  char *tag = NULL;
  if (peek(ctx)->type == TOKEN_IDENT) {
    tag = consume(ctx)->value.ident;
  }

  if (!consume_if(ctx, TOKEN_BRACE_OPEN)) {
    return enum_of(tag, NULL);
  }

  char *name = expect(ctx, TOKEN_IDENT)->value.ident;
  enum_t *head = new_enum(name, 0);
  enum_t *cur = head;

  int value = 1;
  while (consume_if(ctx, TOKEN_COMMA)) {
    if (peek(ctx)->type == TOKEN_BRACE_CLOSE) {
      break;
    }

    name = expect(ctx, TOKEN_IDENT)->value.ident;
    cur->next = new_enum(name, value);
    value++;
    cur = cur->next;
  }

  expect(ctx, TOKEN_BRACE_CLOSE);

  return enum_of(tag, head);
}

type_t *parse_type(parser_ctx_t *ctx) {
  type_t *type;
  switch (peek(ctx)->type) {
  case TOKEN_VOID:
    consume(ctx);
    type = new_type(TYPE_VOID);
    break;
  case TOKEN_CHAR:
    consume(ctx);
    type = new_type(TYPE_CHAR);
    break;
  case TOKEN_INT:
    consume(ctx);
    type = new_type(TYPE_INT);
    break;
  case TOKEN_STRUCT:
  case TOKEN_UNION:
    type = parse_struct_union(ctx);
    break;
  case TOKEN_ENUM:
    type = parse_enum(ctx);
    break;
  case TOKEN_IDENT: {
    token_t *token = consume(ctx);
    typedef_t *typdef = find_typedef(ctx, token->value.ident);
    if (!typdef) {
      error(token->pos, "unknown type: %s\n", token->value.ident);
    }
    type = typdef->type;
    break;
  }
  default:
    error(peek(ctx)->pos, "unknown type: token=%d\n", peek(ctx)->type);
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
  pos_t *pos = peek(ctx)->pos;
  type_t *type = parse_type(ctx);
  char *name = expect(ctx, TOKEN_IDENT)->value.ident;
  type = parse_type_post(ctx, type);

  expr_t *value = NULL;
  if (consume_if(ctx, TOKEN_ASSIGN)) {
    value = parse_expr(ctx);
  }
  expect(ctx, TOKEN_SEMICOLON);

  stmt_t *stmt = new_stmt(STMT_DEFINE, pos);
  stmt->value.define.type = type;
  stmt->value.define.name = name;
  stmt->value.define.value = value;
  return stmt;
}

stmt_case_t *parse_case(parser_ctx_t *ctx) {
  expr_t *value = NULL;
  if (!consume_if(ctx, TOKEN_DEFAULT)) {
    expect(ctx, TOKEN_CASE);
    value = parse_expr(ctx);
  }
  expect(ctx, TOKEN_COLON);

  stmt_list_t *head = NULL;
  stmt_list_t *cur = head;
  while (peek(ctx)->type != TOKEN_CASE && peek(ctx)->type != TOKEN_DEFAULT &&
         peek(ctx)->type != TOKEN_BRACE_CLOSE) {

    if (!head) {
      head = new_stmt_list(parse_stmt(ctx));
      cur = head;
    } else {
      cur->next = new_stmt_list(parse_stmt(ctx));
      cur = cur->next;
    }
  }

  return new_stmt_case(value, head);
}

stmt_t *parse_switch(parser_ctx_t *ctx) {
  pos_t *pos = expect(ctx, TOKEN_SWITCH)->pos;
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

  stmt_t *stmt = new_stmt(STMT_SWITCH, pos);
  stmt->value.switch_.value = value;
  stmt->value.switch_.cases = head;
  stmt->value.switch_.default_case = default_case;
  return stmt;
}

stmt_t *parse_stmt(parser_ctx_t *ctx) {
  pos_t *pos = peek(ctx)->pos;
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
    return new_stmt(STMT_BREAK, pos);
  case TOKEN_CONTINUE:
    consume(ctx);
    expect(ctx, TOKEN_SEMICOLON);
    return new_stmt(STMT_CONTINUE, pos);
  case TOKEN_SWITCH:
    return parse_switch(ctx);
  default:
    if (is_type(ctx, peek(ctx))) {
      return parse_define(ctx);
    } else {
      stmt_t *stmt = new_stmt(STMT_EXPR, pos);
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
    if (consume_if(ctx, TOKEN_VARARG)) {
      break;
    }
    type = parse_type(ctx);
    name = expect(ctx, TOKEN_IDENT)->value.ident;
    cur->next = new_parameter(type, name);
    cur = cur->next;
  }

  return head;
}

global_stmt_t *parse_typedef(parser_ctx_t *ctx) {
  pos_t *pos = expect(ctx, TOKEN_TYPEDEF)->pos;

  type_t *type = parse_type(ctx);
  char *name = expect(ctx, TOKEN_IDENT)->value.ident;
  expect(ctx, TOKEN_SEMICOLON);

  add_typedef(ctx, type, name);

  global_stmt_t *gstmt = new_global_stmt(GSTMT_TYPEDEF, pos);
  gstmt->value.type = type;
  return gstmt;
}

global_stmt_t *parse_global_stmt(parser_ctx_t *ctx) {
  global_stmt_t *gstmt;
  pos_t *pos = peek(ctx)->pos;

  if (peek(ctx)->type == TOKEN_TYPEDEF) {
    return parse_typedef(ctx);
  }

  type_t *type = parse_type(ctx);
  if (peek(ctx)->type != TOKEN_IDENT) {
    expect(ctx, TOKEN_SEMICOLON);
    switch (type->kind) {
    case TYPE_STRUCT:
      gstmt = new_global_stmt(GSTMT_STRUCT, pos);
      break;
    case TYPE_UNION:
      gstmt = new_global_stmt(GSTMT_UNION, pos);
      break;
    case TYPE_ENUM:
      gstmt = new_global_stmt(GSTMT_ENUM, pos);
      break;
    default:
      error(pos, "unexpected type: kind=%d\n", type->kind);
    }
    gstmt->value.type = type;
    return gstmt;
  }

  gstmt = new_global_stmt(GSTMT_FUNC, pos);
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
