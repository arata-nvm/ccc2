#pragma once
#include "parser.h"
#include <stdio.h>

void gen_expr(expr_t *expr, FILE *fp);

void gen_stmt(stmt_t *stmt, FILE *fp);

void gen_code(stmt_t *stmt, FILE *fp);
