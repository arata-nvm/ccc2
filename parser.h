#pragma once
#include "tokenizer.h"
#include "type.h"

typedef struct _typedef_t typedef_t;
struct _typedef_t {
  type_t *type;
  char *name;

  typedef_t *next;
};

typedef struct {
  token_t *cur_token;
  typedef_t *typedefs;
} parser_ctx_t;

typedef enum {
  // special
  EXPR_NUMBER,
  EXPR_STRING,
  EXPR_IDENT,
  EXPR_ASSIGN,
  EXPR_CALL,
  EXPR_MEMBER,

  // unary
  EXPR_REF,
  EXPR_DEREF,
  EXPR_SIZEOF,
  EXPR_NOT,
  EXPR_NEG,

  // binary
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
  EXPR_AND,
  EXPR_OR,
  EXPR_XOR,
  EXPR_SHL,
  EXPR_SHR,
  EXPR_LOGAND,
  EXPR_LOGOR,
} exprtype_t;

int is_unary_expr(exprtype_t type);
int is_binary_expr(exprtype_t type);

typedef struct _argument_t argument_t;
typedef struct _expr_t expr_t;

struct _argument_t {
  expr_t *value;
  argument_t *next;
};

struct _expr_t {
  exprtype_t type;
  pos_t *pos;
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
    struct {
      expr_t *expr;
      char *name;
    } member;
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
  STMT_SWITCH,
} stmttype_t;

typedef struct _stmt_list_t stmt_list_t;
typedef struct _stmt_case_t stmt_case_t;
typedef struct _stmt_t stmt_t;

struct _stmt_list_t {
  stmt_t *stmt;
  stmt_list_t *next;
};

struct _stmt_case_t {
  int value;
  stmt_list_t *body;
  int label;

  stmt_case_t *next;
};

struct _stmt_t {
  stmttype_t type;
  pos_t *pos;
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
    struct {
      expr_t *value;
      stmt_case_t *cases;
      stmt_case_t *default_case;
    } switch_;
  } value;
};

typedef enum {
  GSTMT_FUNC,
  GSTMT_FUNC_DECL,
  GSTMT_STRUCT,
  GSTMT_TYPEDEF,
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
  pos_t *pos;
  union {
    struct {
      type_t *ret_type;
      char *name;
      parameter_t *params;
      stmt_t *body;
    } func;
    type_t *struct_;
  } value;

  global_stmt_t *next;
};

typedef struct {
  global_stmt_t *body;
} program_t;

program_t *parse(token_t *token);
