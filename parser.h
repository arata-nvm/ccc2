#pragma once
#include "tokenizer.h"
#include "type.h"

typedef struct {
  token_t *cur_token;
} token_cursor_t;

typedef enum {
  EXPR_NUMBER,
  EXPR_STRING,
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
  EXPR_CALL,
  EXPR_REF,
  EXPR_DEREF,
  EXPR_SIZEOF,
  EXPR_AND,
  EXPR_OR,
  EXPR_NOT,
  EXPR_XOR,
  EXPR_NEG,
  EXPR_SHL,
  EXPR_SHR,
  EXPR_LOGAND,
  EXPR_LOGOR,
} exprtype_t;

typedef struct _argument_t argument_t;
typedef struct _expr_t expr_t;

struct _argument_t {
  expr_t *value;
  argument_t *next;
};

struct _expr_t {
  exprtype_t type;
  union {
    int number;
    char *string;
    char *ident;
    expr_t *unary;
    struct {
      expr_t *lhs;
      expr_t *rhs;
    } binary;
    struct {
      expr_t *dst;
      expr_t *src;
    } assign;
    struct {
      char *name;
      argument_t *args;
    } call;
  } value;
};

typedef enum {
  STMT_EXPR,
  STMT_RETURN,
  STMT_IF,
  STMT_WHILE,
  STMT_FOR,
  STMT_BLOCK,
  STMT_DEFINE,
  STMT_BREAK,
  STMT_CONTINUE,
} stmttype_t;

typedef struct _stmt_list_t stmt_list_t;
typedef struct _stmt_t stmt_t;

struct _stmt_list_t {
  stmt_t *stmt;
  stmt_list_t *next;
};

struct _stmt_t {
  stmttype_t type;
  union {
    expr_t *expr;
    expr_t *ret;
    struct {
      expr_t *cond;
      stmt_t *then_;
      stmt_t *else_;
    } if_;
    struct {
      expr_t *cond;
      stmt_t *body;
    } while_;
    struct {
      stmt_t *init;
      expr_t *cond;
      expr_t *loop;
      stmt_t *body;
    } for_;
    stmt_list_t *block;
    struct {
      type_t *type;
      char *name;
      expr_t *value;
    } define;
  } value;
};

typedef enum {
  GSTMT_FUNC,
  GSTMT_FUNC_DECL,
} global_stmttype_t;

typedef struct _parameter_t parameter_t;
typedef struct _global_stmt_t global_stmt_t;

struct _parameter_t {
  type_t *type;
  char *name;

  parameter_t *next;
};

struct _global_stmt_t {
  global_stmttype_t type;
  union {
    struct {
      type_t *ret_type;
      char *name;
      parameter_t *params;
      stmt_t *body;
    } func;
  } value;

  global_stmt_t *next;
};

global_stmt_t *parse(token_t *token);
