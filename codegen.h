#pragma once
#include "parser.h"
#include "type.h"
#include <stdio.h>

typedef struct _variable_t variable_t;
struct _variable_t {
  type_t *type;
  char *name;
  int offset;

  variable_t *next;
};

typedef struct {
  FILE *fp;
  variable_t *variables;
  int cur_offset;
  int cur_label;
  char *cur_func_name;
} codegen_ctx_t;

void gen_code(global_stmt_t *gstmt, FILE *fp);
