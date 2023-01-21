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

typedef struct _function_t function_t;
struct _function_t {
  type_t *ret_type;
  char *name;

  function_t *next;
};

typedef struct _string_t string_t;
struct _string_t {
  char *string;
  string_t *next;
};

typedef struct _loop_t loop_t;
struct _loop_t {
  int break_label;
  int continue_label;

  loop_t *next;
};

typedef struct _defined_type_t defined_type_t;
struct _defined_type_t {
  type_t *type;

  defined_type_t *next;
};

typedef struct {
  FILE *fp;

  variable_t *variables;
  function_t *functions;
  string_t *strings;
  loop_t *loops;
  defined_type_t *types;
  enum_t *enums;

  char *cur_func_name;

  int cur_offset;
  int cur_label;
  int cur_string;
} codegen_ctx_t;

void gen_code(program_t *program, FILE *fp);
