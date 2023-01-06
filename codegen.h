#pragma once
#include "parser.h"
#include <stdio.h>

typedef struct {
  FILE *fp;
} codegen_ctx_t;

codegen_ctx_t *new_codegen_ctx(FILE *fp);

void gen_expr(expr_t *expr, codegen_ctx_t *ctx);

void gen_stmt(stmt_t *stmt, codegen_ctx_t *ctx);

void gen_code(stmt_t *stmt, FILE *fp);
