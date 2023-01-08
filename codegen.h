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

typedef struct _string_t string_t;
struct _string_t {
  char *string;
  string_t *next;
};

typedef struct {
  FILE *fp;

  variable_t *variables;
  string_t *strings;

  char *cur_func_name;

  int cur_offset;
  int cur_label;
  int cur_string;
} codegen_ctx_t;

void gen_code(global_stmt_t *gstmt, FILE *fp);
