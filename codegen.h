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

void gen_code(stmt_t *stmt, FILE *fp);
