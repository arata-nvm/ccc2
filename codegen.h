#pragma once
#include "parser.h"
#include <stdio.h>

typedef struct _variable_t variable_t;
struct _variable_t {
  char *name;
  int offset;

  variable_t *next;
};

typedef struct {
  FILE *fp;
  variable_t *variables;
  int cur_offset;
  int cur_label;
} codegen_ctx_t;

codegen_ctx_t *new_codegen_ctx(FILE *fp);

void add_variable(codegen_ctx_t *ctx, char *name, int offset);

// returns -1 if variable 'name' is not found
int find_variable(codegen_ctx_t *ctx, char *name);

void gen_expr(expr_t *expr, codegen_ctx_t *ctx);

void gen_stmt(stmt_t *stmt, codegen_ctx_t *ctx);

void gen_code(stmt_t *stmt, FILE *fp);
