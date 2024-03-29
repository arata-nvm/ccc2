#include "codegen.h"
#include "error.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

char *arg_regs[8];

void init_arg_regs() {
  arg_regs[0] = "x0";
  arg_regs[1] = "x1";
  arg_regs[2] = "x2";
  arg_regs[3] = "x3";
  arg_regs[4] = "x4";
  arg_regs[5] = "x5";
  arg_regs[6] = "x6";
  arg_regs[7] = "x7";
}

void gen_expr(codegen_ctx_t *ctx, expr_t *expr);
void gen_stmt(codegen_ctx_t *ctx, stmt_t *stmt);

void push_scope(codegen_ctx_t *ctx) {
  var_scope_t *var_scope = calloc(1, sizeof(var_scope_t));
  var_scope->parent = ctx->var_scopes;
  ctx->var_scopes = var_scope;

  type_scope_t *type_scope = calloc(1, sizeof(type_scope_t));
  type_scope->parent = ctx->type_scopes;
  ctx->type_scopes = type_scope;
}

void pop_scope(codegen_ctx_t *ctx) {
  ctx->var_scopes = ctx->var_scopes->parent;
  ctx->type_scopes = ctx->type_scopes->parent;
}

codegen_ctx_t *new_codegen_ctx(char *in_filepath, FILE *out_fp,
                               global_var_t *globals) {
  codegen_ctx_t *ctx = calloc(1, sizeof(codegen_ctx_t));
  ctx->in_filepath = in_filepath;
  ctx->out_fp = out_fp;
  ctx->cur_offset = 16;
  ctx->globals = globals;
  push_scope(ctx);
  return ctx;
}

void init_ctx(codegen_ctx_t *ctx, char *func_name) {
  ctx->cur_offset = 16;
  ctx->cur_label = 0;
  ctx->cur_func_name = func_name;
}

variable_t *add_variable(codegen_ctx_t *ctx, type_t *type, char *name) {
  variable_t *variable = calloc(1, sizeof(variable_t));
  variable->type = type;
  variable->name = name;

  variable->offset = ctx->cur_offset;
  ctx->cur_offset =
      align_to(ctx->cur_offset + type_size(type), type_align(type));

  var_scope_t *cur_scope = ctx->var_scopes;
  variable->next = cur_scope->variables;
  cur_scope->variables = variable;

  return variable;
}

variable_t *find_variable_in(codegen_ctx_t *ctx, char *name,
                             var_scope_t *scope) {
  variable_t *cur = scope->variables;
  while (cur) {
    if (!strcmp(cur->name, name)) {
      return cur;
    }
    cur = cur->next;
  }

  return NULL;
}

variable_t *find_variable(codegen_ctx_t *ctx, char *name) {
  var_scope_t *cur_scope = ctx->var_scopes;

  while (cur_scope) {
    variable_t *variable = find_variable_in(ctx, name, cur_scope);
    if (variable) {
      return variable;
    }

    cur_scope = cur_scope->parent;
  }

  return NULL;
}

int is_variable_already_defined(codegen_ctx_t *ctx, char *name) {
  return find_variable_in(ctx, name, ctx->var_scopes) != NULL;
}

function_t *add_function(codegen_ctx_t *ctx, type_t *ret_type, char *name) {
  function_t *function = calloc(1, sizeof(function_t));
  function->ret_type = ret_type;
  function->name = name;

  function->next = ctx->functions;
  ctx->functions = function;

  return function;
}

function_t *find_function(codegen_ctx_t *ctx, char *name) {
  function_t *cur = ctx->functions;
  while (cur) {
    if (!strcmp(cur->name, name)) {
      return cur;
    }
    cur = cur->next;
  }

  return NULL;
}

int add_string(codegen_ctx_t *ctx, char *string) {
  string_t *str = calloc(1, sizeof(string_t));
  str->string = string;

  str->next = ctx->strings;
  ctx->strings = str;

  ctx->cur_string++;
  return ctx->cur_string;
}

void push_loop(codegen_ctx_t *ctx, int break_label, int continue_label) {
  loop_t *loop = calloc(1, sizeof(loop_t));
  loop->break_label = break_label;
  loop->continue_label = continue_label;

  loop->next = ctx->loops;
  ctx->loops = loop;
}

loop_t *cur_loop(codegen_ctx_t *ctx) { return ctx->loops; }

void pop_loop(codegen_ctx_t *ctx) { ctx->loops = ctx->loops->next; }

void add_type(codegen_ctx_t *ctx, type_t *type) {
  defined_type_t *defined_type = calloc(1, sizeof(defined_type_t));
  defined_type->type = type;

  type_scope_t *cur_scope = ctx->type_scopes;
  defined_type->next = cur_scope->types;
  cur_scope->types = defined_type;
}

type_t *find_type_in(codegen_ctx_t *ctx, char *tag, type_scope_t *scope) {
  defined_type_t *cur = scope->types;
  while (cur) {
    char *cur_tag = cur->type->value.struct_union.tag;
    if (cur_tag && !strcmp(cur_tag, tag)) {
      return cur->type;
    }
    cur = cur->next;
  }

  return NULL;
}

type_t *find_type(codegen_ctx_t *ctx, char *tag) {
  type_scope_t *cur_scope = ctx->type_scopes;

  while (cur_scope) {
    type_t *type = find_type_in(ctx, tag, cur_scope);
    if (type) {
      return type;
    }

    cur_scope = cur_scope->parent;
  }

  return NULL;
}

type_t *complete_type(codegen_ctx_t *ctx, type_t *type) {
  if (!is_incomlete(type)) {
    return type;
  }

  switch (type->kind) {
  case TYPE_VOID:
  case TYPE_CHAR:
  case TYPE_INT:
    return type;
  case TYPE_PTR:
    type->value.ptr = complete_type(ctx, type->value.ptr);
    return type;
  case TYPE_ARRAY:
    type->value.array.elm = complete_type(ctx, type->value.array.elm);
    return type;
  case TYPE_STRUCT:
  case TYPE_UNION: {
    type_t *completed_type = find_type(ctx, type->value.struct_union.tag);
    if (completed_type) {
      return completed_type;
    }
    return type;
  }
  case TYPE_ENUM: {
    type_t *completed_type = find_type(ctx, type->value.enum_.tag);
    if (completed_type) {
      return completed_type;
    }
    return type;
  }
  }
}

void append_enums(codegen_ctx_t *ctx, enum_t *enums) {
  if (ctx->enums == NULL) {
    ctx->enums = enums;
    return;
  }

  enum_t *cur = ctx->enums;
  while (cur->next) {
    cur = cur->next;
  }

  cur->next = enums;
}

int find_enum(codegen_ctx_t *ctx, char *name, int *value) {
  enum_t *cur = ctx->enums;
  while (cur) {
    if (!strcmp(cur->name, name)) {
      *value = cur->value;
      return 1;
    }
    cur = cur->next;
  }

  return 0;
}

global_var_t *find_global(codegen_ctx_t *ctx, char *name) {
  global_var_t *cur = ctx->globals;
  while (cur) {
    if (!strcmp(cur->name, name)) {
      return cur;
    }
    cur = cur->next;
  }

  return NULL;
}

int next_label(codegen_ctx_t *ctx) {
  ctx->cur_label++;
  return ctx->cur_label;
}

int eval_const_expr(codegen_ctx_t *ctx, expr_t *expr) {
  switch (expr->type) {
  case EXPR_CHAR:
    return expr->value.char_;
  case EXPR_NUMBER:
    return expr->value.number;
  case EXPR_IDENT: {
    int enum_value;
    if (find_enum(ctx, expr->value.ident, &enum_value)) {
      return enum_value;
    }
    error(expr->pos, "unknown enum '%s'\n", expr->value.ident);
  }
  default:
    error(expr->pos, "unimplemented const expression\n");
  }
}

#ifdef __GNUC__
#define GEN_NAME gen
void GEN_NAME(codegen_ctx_t *ctx, char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(ctx->out_fp, format, args);
  va_end(args);
}
#else
void gen(codegen_ctx_t *ctx, char *format, void *a1, void *a2, void *a3,
         void *a4) {
  fprintf(ctx->out_fp, format, a1, a2, a3, a4);
}
#endif

void gen_label(codegen_ctx_t *ctx, int label) {
  gen(ctx, ".L.%s.%d:\n", ctx->cur_func_name, label);
}

void gen_branch(codegen_ctx_t *ctx, char *op, int label) {
  gen(ctx, "  %s .L.%s.%d\n", op, ctx->cur_func_name, label);
}

void gen_push(codegen_ctx_t *ctx, char *reg) {
  gen(ctx, "  str %s, [sp, -16]!\n", reg);
}

void gen_pop(codegen_ctx_t *ctx, char *reg) {
  gen(ctx, "  ldr %s, [sp], 16\n", reg);
}

void gen_load(codegen_ctx_t *ctx, type_t *type, pos_t *pos) {
  gen_pop(ctx, "x8");
  switch (type_size(type)) {
  case 1:
    gen(ctx, "  ldrb w8, [x8]\n");
    gen(ctx, "  sxtb x8, w8\n");
    break;
  case 4:
    gen(ctx, "  ldr w8, [x8]\n");
    gen(ctx, "  sxtw x8, w8\n");
    break;
  case 8:
    gen(ctx, "  ldr x8, [x8]\n");
    break;
  default:
    error(pos, "cannot load: type=%d\n", type->kind);
  }
  gen_push(ctx, "x8");
}

void gen_store(codegen_ctx_t *ctx, type_t *type, pos_t *pos) {
  gen_pop(ctx, "x8"); // dst
  gen_pop(ctx, "x9"); // src
  switch (type_size(type)) {
  case 1:
    gen(ctx, "  strb w9, [x8]\n");
    break;
  case 4:
    gen(ctx, "  str w9, [x8]\n");
    break;
  case 8:
    gen(ctx, "  str x9, [x8]\n");
    break;
  default:
    error(pos, "cannot store: type=%d\n", type->kind);
  }
}

void gen_var_addr(codegen_ctx_t *ctx, variable_t *var) {
  gen(ctx, "  add x8, x29, %d\n", var->offset);
  gen_push(ctx, "x8");
}

void gen_str_addr(codegen_ctx_t *ctx, int str_index) {
  gen(ctx, "  adrp x8, .L.str.%d\n", str_index);
  gen(ctx, "  add x8, x8, :lo12:.L.str.%d\n", str_index);
  gen_push(ctx, "x8");
}

void gen_global_addr(codegen_ctx_t *ctx, global_var_t *global) {
  gen(ctx, "  adrp x8, %s\n", global->name);
  gen(ctx, "  add x8, x8, :lo12:%s\n", global->name);
  gen_push(ctx, "x8");
}

type_t *infer_expr_type(codegen_ctx_t *ctx, expr_t *expr) {
  switch (expr->type) {
  case EXPR_CHAR:
    return new_type(TYPE_CHAR);
  case EXPR_NUMBER:
    return new_type(TYPE_INT);
  case EXPR_STRING:
    return ptr_to(new_type(TYPE_CHAR));
  case EXPR_IDENT: {
    variable_t *var = find_variable(ctx, expr->value.ident);
    if (var != NULL) {
      return var->type;
    }

    global_var_t *global = find_global(ctx, expr->value.ident);
    if (global != NULL) {
      return complete_type(ctx, global->type);
    }

    int enum_value;
    if (find_enum(ctx, expr->value.ident, &enum_value)) {
      return new_type(TYPE_INT);
    }

    error(expr->pos, "unknown variable '%s'\n", expr->value.ident);
  }
  case EXPR_ADD: {
    type_t *lhs_type = infer_expr_type(ctx, expr->value.binary.lhs);
    type_t *rhs_type = infer_expr_type(ctx, expr->value.binary.rhs);
    if (is_integer(lhs_type) && is_integer(rhs_type)) {
      return lhs_type;
    } else if (is_ptr(lhs_type) && is_integer(rhs_type)) {
      return lhs_type;
    } else if (is_integer(lhs_type) && is_ptr(rhs_type)) {
      return rhs_type;
    }
    error(expr->pos, "invalid add operation: lhs=%d, rhs=%d\n",
          pos_to_string(expr->pos), lhs_type->kind, rhs_type->kind);
  }
  case EXPR_SUB: {
    type_t *lhs_type = infer_expr_type(ctx, expr->value.binary.lhs);
    type_t *rhs_type = infer_expr_type(ctx, expr->value.binary.rhs);
    if (is_integer(lhs_type) && is_integer(rhs_type)) {
      return lhs_type;
    } else if (is_ptr(lhs_type) && is_integer(rhs_type)) {
      return lhs_type;
    } else if (is_ptr(lhs_type) && is_ptr(rhs_type)) {
      return new_type(TYPE_INT);
    }
    error(expr->pos, "invalid sub operation: lhs=%d, rhs=%d\n",
          pos_to_string(expr->pos), lhs_type->kind, rhs_type->kind);
  }
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
  case EXPR_NOT:
  case EXPR_XOR:
  case EXPR_NEG:
  case EXPR_SHL:
  case EXPR_SHR:
  case EXPR_LOGAND:
  case EXPR_LOGOR:
    return new_type(TYPE_INT); // TODO
  case EXPR_ASSIGN:
    return infer_expr_type(ctx, expr->value.assign.dst);
  case EXPR_CALL: {
    function_t *func = find_function(ctx, expr->value.call.name);
    if (func != NULL) {
      return func->ret_type;
    }

    error(expr->pos, "unknown function '%s'\n", expr->value.call.name);
  }

    return new_type(TYPE_INT); // TODO
  case EXPR_REF:
    return ptr_to(infer_expr_type(ctx, expr->value.unary));
  case EXPR_DEREF: {
    type_t *ptr_type = infer_expr_type(ctx, expr->value.unary);
    return type_deref(ptr_type);
  }
  case EXPR_SIZEOF:
    return new_type(TYPE_INT);
  case EXPR_MEMBER: {
    expr_t *mexpr = expr->value.member.expr;
    char *name = expr->value.member.name;

    type_t *mtype = infer_expr_type(ctx, mexpr);
    struct_member_t *member = find_member(mtype, name);
    if (member == NULL) {
      error(expr->pos, "unknown member: type=%d, name=%s\n", mtype->kind, name);
    }

    return complete_type(ctx, member->type);
  }
  case EXPR_INC_PRE:
  case EXPR_INC_POST:
  case EXPR_DEC_PRE:
  case EXPR_DEC_POST:
    return infer_expr_type(ctx, expr->value.unary);
  }
  error(expr->pos, "unreachable\n");
}

void gen_lvalue(codegen_ctx_t *ctx, expr_t *expr) {
  switch (expr->type) {
  case EXPR_IDENT: {
    variable_t *var = find_variable(ctx, expr->value.ident);
    if (var != NULL) {
      gen_var_addr(ctx, var);
      break;
    }

    global_var_t *global = find_global(ctx, expr->value.ident);
    if (global != NULL) {
      gen_global_addr(ctx, global);
      break;
    }

    error(expr->pos, "unknown variable '%s'\n", expr->value.ident);
    break;
  }
  case EXPR_DEREF:
    gen_expr(ctx, expr->value.unary);
    break;
  case EXPR_MEMBER: {
    expr_t *mexpr = expr->value.member.expr;
    char *name = expr->value.member.name;

    type_t *mtype = infer_expr_type(ctx, mexpr);
    struct_member_t *member = find_member(mtype, name);
    if (member == NULL) {
      error(expr->pos, "unknown member: type=%d, name=%s\n", mtype->kind, name);
    }

    gen_lvalue(ctx, mexpr);
    gen_pop(ctx, "x8");
    gen(ctx, "  add x8, x8, %d\n", member->offset);
    gen_push(ctx, "x8");
    break;
  }
  default:
    error(expr->pos, "cannot generate lvalue: expr=%d\n", expr->type);
  }
}

void gen_special_expr(codegen_ctx_t *ctx, expr_t *expr) {
  switch (expr->type) {
  case EXPR_CHAR:
    gen(ctx, "  mov x8, %d\n", expr->value.char_);
    gen_push(ctx, "x8");
    break;
  case EXPR_NUMBER:
    gen(ctx, "  mov x8, %d\n", expr->value.number);
    gen_push(ctx, "x8");
    break;
  case EXPR_STRING: {
    int str_index = add_string(ctx, expr->value.string);
    gen_str_addr(ctx, str_index);
    break;
  }
  case EXPR_IDENT: {
    variable_t *var = find_variable(ctx, expr->value.ident);
    if (var != NULL) {
      gen_var_addr(ctx, var);
      if (var->type->kind != TYPE_ARRAY) {
        gen_load(ctx, var->type, expr->pos);
      }
      break;
    }

    global_var_t *global = find_global(ctx, expr->value.ident);
    if (global != NULL) {
      gen_global_addr(ctx, global);
      if (global->type->kind != TYPE_ARRAY) {
        gen_load(ctx, global->type, expr->pos);
      }
      break;
    }

    int enum_value;
    if (find_enum(ctx, expr->value.ident, &enum_value)) {
      gen(ctx, "  mov x8, %d\n", enum_value);
      gen_push(ctx, "x8");
      break;
    }

    error(expr->pos, "unknown variable '%s'\n", expr->value.ident);
    break;
  }
  case EXPR_ASSIGN:
    gen_expr(ctx, expr->value.assign.src);
    gen_lvalue(ctx, expr->value.assign.dst);
    gen_store(ctx, infer_expr_type(ctx, expr), expr->pos);
    gen_push(ctx, "x8"); // FIXME
    break;
  case EXPR_CALL: {
    int i = 0;
    argument_t *cur_arg = expr->value.call.args;
    while (cur_arg) {
      if (i > 7) {
        error(expr->pos, "cannot use > 7 arguments\n");
      }

      gen_expr(ctx, cur_arg->value);

      cur_arg = cur_arg->next;
      i++;
    }

    int j = 0;
    while (j < i) {
      gen_pop(ctx, arg_regs[i - j - 1]);
      j++;
    }

    gen(ctx, "  bl %s\n", expr->value.ident);
    gen_push(ctx, "x0");
    break;
  }
  case EXPR_MEMBER:
    gen_lvalue(ctx, expr);
    gen_load(ctx, infer_expr_type(ctx, expr), expr->pos);
    break;
  default:
    error(expr->pos, "unreachable: expr=%d\n", expr->type);
  }
}

void gen_unary_expr(codegen_ctx_t *ctx, expr_t *expr) {
  switch (expr->type) {
  case EXPR_REF:
    gen_lvalue(ctx, expr->value.unary);
    break;
  case EXPR_DEREF:
    gen_expr(ctx, expr->value.unary);
    gen_load(ctx, infer_expr_type(ctx, expr), expr->pos);
    break;
  case EXPR_SIZEOF: {
    type_t *type = expr->value.sizeof_.type;
    if (!type) {
      type = infer_expr_type(ctx, expr->value.sizeof_.expr);
    }
    type = complete_type(ctx, type);
    gen(ctx, "  mov x8, %d\n", type_size(type));
    gen_push(ctx, "x8");
    break;
  }
  case EXPR_NOT:
    gen_expr(ctx, expr->value.unary);
    gen_pop(ctx, "x8");
    gen(ctx, "  mvn x8, x8\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_NEG:
    gen_expr(ctx, expr->value.unary);
    gen_pop(ctx, "x8");
    gen(ctx, "  subs x8, x8, 0\n");
    gen(ctx, "  cset x8, eq\n");
    gen_push(ctx, "x8"); // dup
    break;
  case EXPR_INC_PRE:
    gen_expr(ctx, expr->value.unary);
    gen_pop(ctx, "x8");
    gen(ctx, "  add x8, x8, 1\n"); // TODO
    gen_push(ctx, "x8");           // dup
    gen_push(ctx, "x8");

    gen_lvalue(ctx, expr->value.unary);
    gen_store(ctx, infer_expr_type(ctx, expr), expr->pos);
    break;
  case EXPR_INC_POST:
    gen_expr(ctx, expr->value.unary);
    gen_pop(ctx, "x8");
    gen_push(ctx, "x8");
    gen(ctx, "  add x8, x8, 1\n"); // TODO
    gen_push(ctx, "x8");

    gen_lvalue(ctx, expr->value.unary);
    gen_store(ctx, infer_expr_type(ctx, expr), expr->pos);
    break;
  case EXPR_DEC_PRE:
    gen_expr(ctx, expr->value.unary);
    gen_pop(ctx, "x8");
    gen(ctx, "  sub x8, x8, 1\n"); // TODO
    gen_push(ctx, "x8");
    gen_push(ctx, "x8");

    gen_lvalue(ctx, expr->value.unary);
    gen_store(ctx, infer_expr_type(ctx, expr), expr->pos);
    break;
  case EXPR_DEC_POST:
    gen_expr(ctx, expr->value.unary);
    gen_pop(ctx, "x8");
    gen_push(ctx, "x8");           // dup
    gen(ctx, "  sub x8, x8, 1\n"); // TODO
    gen_push(ctx, "x8");

    gen_lvalue(ctx, expr->value.unary);
    gen_store(ctx, infer_expr_type(ctx, expr), expr->pos);
    break;
  default:
    error(expr->pos, "unreachable: expr=%d\n", expr->type);
  }
}

void gen_binary_expr(codegen_ctx_t *ctx, expr_t *expr) {
  switch (expr->type) {
  case EXPR_LOGAND: {
    int skip_label = next_label(ctx);
    gen_expr(ctx, expr->value.binary.lhs);
    gen_pop(ctx, "x8");
    gen_push(ctx, "x8"); // dup
    gen(ctx, "  subs x8, x8, 0\n");
    gen_branch(ctx, "beq", skip_label);
    gen_expr(ctx, expr->value.binary.rhs);
    gen_label(ctx, skip_label);
    return;
  }
  case EXPR_LOGOR: {
    int skip_label = next_label(ctx);
    gen_expr(ctx, expr->value.binary.lhs);
    gen_pop(ctx, "x8");
    gen_push(ctx, "x8"); // dup
    gen(ctx, "  subs x8, x8, 0\n");
    gen_branch(ctx, "bne", skip_label);
    gen_expr(ctx, expr->value.binary.rhs);
    gen_label(ctx, skip_label);
    return;
  }
  default:
    break;
  }

  gen_expr(ctx, expr->value.binary.lhs);
  gen_expr(ctx, expr->value.binary.rhs);
  gen_pop(ctx, "x9");
  gen_pop(ctx, "x8");

  switch (expr->type) {
  case EXPR_ADD: {
    type_t *lhs_type = infer_expr_type(ctx, expr->value.binary.lhs);
    type_t *rhs_type = infer_expr_type(ctx, expr->value.binary.rhs);
    if (is_integer(lhs_type) && is_integer(rhs_type)) {
      // do nothing
    } else if (is_ptr(lhs_type) && is_integer(rhs_type)) {
      gen(ctx, "  mov x10, %d\n", type_size(type_deref(lhs_type)));
      gen(ctx, "  mul x9, x9, x10\n");
    } else if (is_integer(lhs_type) && is_ptr(rhs_type)) {
      gen(ctx, "  mov x10, %d\n", type_size(type_deref(rhs_type)));
      gen(ctx, "  mul x8, x8, x10\n");
    } else {
      error(expr->pos, "invalid add operation: lhs=%d, rhs=%d\n",
            lhs_type->kind, rhs_type->kind);
    }
    gen(ctx, "  add x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  }
  case EXPR_SUB: {
    type_t *lhs_type = infer_expr_type(ctx, expr->value.binary.lhs);
    type_t *rhs_type = infer_expr_type(ctx, expr->value.binary.rhs);
    if (is_integer(lhs_type) && is_integer(rhs_type)) {
      gen(ctx, "  sub x8, x8, x9\n");
    } else if (is_ptr(lhs_type) && is_integer(rhs_type)) {
      gen(ctx, "  mov x10, %d\n", type_size(type_deref(lhs_type)));
      gen(ctx, "  mul x9, x9, x10\n");
      gen(ctx, "  sub x8, x8, x9\n");
    } else if (is_ptr(lhs_type) && is_ptr(rhs_type)) {
      gen(ctx, "  sub x8, x8, x9\n");
      gen(ctx, "  mov x9, %d\n", type_size(type_deref(lhs_type)));
      gen(ctx, "  udiv x8, x8, x9\n");
    } else {
      error(expr->pos, "invalid a dd operation: lhs=%d, rhs=%d\n",
            lhs_type->kind, rhs_type->kind);
    }
    gen_push(ctx, "x8");
    break;
  }
  case EXPR_MUL:
    gen(ctx, "  mul x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_DIV:
    gen(ctx, "  sdiv x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_REM:
    gen(ctx, "  sdiv x2, x8, x9\n");
    gen(ctx, "  msub x8, x9, x2, x8\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_LT:
    gen(ctx, "  subs x8, x8, x9\n");
    gen(ctx, "  cset x8, lt\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_LE:
    gen(ctx, "  subs x8, x8, x9\n");
    gen(ctx, "  cset x8, le\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_GT:
    gen(ctx, "  subs x8, x8, x9\n");
    gen(ctx, "  cset x8, gt\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_GE:
    gen(ctx, "  subs x8, x8, x9\n");
    gen(ctx, "  cset x8, ge\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_EQ:
    gen(ctx, "  subs x8, x8, x9\n");
    gen(ctx, "  cset x8, eq\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_NE:
    gen(ctx, "  subs x8, x8, x9\n");
    gen(ctx, "  cset x8, ne\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_AND:
    gen(ctx, "  and x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_OR:
    gen(ctx, "  orr x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_XOR:
    gen(ctx, "  eor x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_SHL:
    gen(ctx, "  lsl x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_SHR:
    gen(ctx, "  asr x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  default:
    error(expr->pos, "unreachab le: expr=%d\n", expr->type);
  }
}

void gen_expr(codegen_ctx_t *ctx, expr_t *expr) {
  if (is_unary_expr(expr->type)) {
    gen_unary_expr(ctx, expr);
  } else if (is_binary_expr(expr->type)) {
    gen_binary_expr(ctx, expr);
  } else {
    gen_special_expr(ctx, expr);
  }
}

void gen_stmt_list(codegen_ctx_t *ctx, stmt_list_t *list) {
  while (list) {
    gen_stmt(ctx, list->stmt);
    list = list->next;
  }
}

void gen_stmt(codegen_ctx_t *ctx, stmt_t *stmt) {
  gen(ctx, ".loc 1 %d %d\n", stmt->pos->line, stmt->pos->column);
  switch (stmt->type) {
  case STMT_EXPR:
    gen_expr(ctx, stmt->value.expr);
    gen_pop(ctx, "x8"); // pop expr value
    break;
  case STMT_RETURN:
    if (stmt->value.ret) {
      gen_expr(ctx, stmt->value.ret);
    } else {
      gen_push(ctx, "x8"); // push dummy value
    }
    gen(ctx, "  b .L.%s.ret\n", ctx->cur_func_name);
    break;
  case STMT_IF: {
    int else_label = next_label(ctx);
    if (stmt->value.if_.else_) {
      int merge_label = next_label(ctx);
      gen_expr(ctx, stmt->value.if_.cond);
      gen_pop(ctx, "x8");
      gen(ctx, "  subs x8, x8, 0\n");
      gen_branch(ctx, "beq", else_label);

      gen_stmt(ctx, stmt->value.if_.then_);
      gen_branch(ctx, "b", merge_label);

      gen_label(ctx, else_label);
      gen_stmt(ctx, stmt->value.if_.else_);

      gen_label(ctx, merge_label);
    } else {
      gen_expr(ctx, stmt->value.if_.cond);
      gen_pop(ctx, "x8");
      gen(ctx, "  subs x8, x8, 0\n");
      gen_branch(ctx, "beq", else_label);

      gen_stmt(ctx, stmt->value.if_.then_);

      gen_label(ctx, else_label);
    }

    break;
  }
  case STMT_WHILE: {
    int cond_label = next_label(ctx);
    int end_label = next_label(ctx);
    push_loop(ctx, end_label, cond_label);

    gen_label(ctx, cond_label);
    gen_expr(ctx, stmt->value.while_.cond);
    gen_pop(ctx, "x8");
    gen(ctx, "  subs x8, x8, 0\n");
    gen_branch(ctx, "beq", end_label);

    gen_stmt(ctx, stmt->value.while_.body);
    gen_branch(ctx, "b", cond_label);

    gen_label(ctx, end_label);
    pop_loop(ctx);
    break;
  }
  case STMT_FOR: {
    int cond_label = next_label(ctx);
    int end_label = next_label(ctx);
    int loop_label = next_label(ctx);
    push_loop(ctx, end_label, loop_label);

    if (stmt->value.for_.init) {
      gen_stmt(ctx, stmt->value.for_.init);
    }

    gen_label(ctx, cond_label);
    if (stmt->value.for_.cond) {
      gen_expr(ctx, stmt->value.for_.cond);
      gen_pop(ctx, "x8");
      gen(ctx, "  subs x8, x8, 0\n");
      gen_branch(ctx, "beq", end_label);
    }

    gen_stmt(ctx, stmt->value.for_.body);

    gen_label(ctx, loop_label);
    if (stmt->value.for_.loop) {
      gen_expr(ctx, stmt->value.for_.loop);
    }
    gen_branch(ctx, "b", cond_label);

    gen_label(ctx, end_label);
    pop_loop(ctx);
    break;
  }
  case STMT_BLOCK:
    push_scope(ctx);
    gen_stmt_list(ctx, stmt->value.block);
    pop_scope(ctx);
    break;
  case STMT_DEFINE: {
    char *name = stmt->value.define.name;
    if (is_variable_already_defined(ctx, name)) {
      error(stmt->pos, "variable '%s' already defined\n", name);
    }
    type_t *type = stmt->value.define.type;
    type = complete_type(ctx, type);
    variable_t *var = add_variable(ctx, type, name);
    if (stmt->value.define.value) {
      gen_expr(ctx, stmt->value.define.value);
      gen_var_addr(ctx, var);
      gen_store(ctx, var->type, stmt->pos);
    }
    break;
  }
  case STMT_BREAK:
    gen_branch(ctx, "b", cur_loop(ctx)->break_label);
    break;
  case STMT_CONTINUE:
    gen_branch(ctx, "b", cur_loop(ctx)->continue_label);
    break;
  case STMT_SWITCH: {
    int merge_label = next_label(ctx);
    push_loop(ctx, merge_label, -1);

    gen_expr(ctx, stmt->value.switch_.value);
    gen_pop(ctx, "x8");
    stmt_case_t *cur_case = stmt->value.switch_.cases;
    while (cur_case) {
      cur_case->label = next_label(ctx);
      int value = eval_const_expr(ctx, cur_case->value);
      gen(ctx, "  cmp x8, %d\n", value);
      gen_branch(ctx, "beq", cur_case->label);

      cur_case = cur_case->next;
    }

    stmt_case_t *default_case = stmt->value.switch_.default_case;
    if (default_case) {
      default_case->label = next_label(ctx);
      gen_branch(ctx, "b", default_case->label);
    }
    gen_branch(ctx, "b", merge_label);

    cur_case = stmt->value.switch_.cases;
    while (cur_case) {
      gen_label(ctx, cur_case->label);
      gen_stmt_list(ctx, cur_case->body);

      cur_case = cur_case->next;
    }

    if (default_case) {
      gen_label(ctx, default_case->label);
      gen_stmt_list(ctx, default_case->body);
    }

    gen_label(ctx, merge_label);
    pop_loop(ctx);
    break;
  }
  }
}

void gen_func_parameter(codegen_ctx_t *ctx, parameter_t *params, pos_t *pos) {
  int i = 0;
  while (params) {
    if (i > 7) {
      error(pos, "cannot use > 7 arguments\n");
    }

    type_t *type = params->type;
    type = complete_type(ctx, type);

    variable_t *var = add_variable(ctx, type, params->name);
    gen_push(ctx, arg_regs[i]);
    gen_var_addr(ctx, var);
    gen_store(ctx, var->type, pos);

    params = params->next;
    i++;
  }
}

void gen_global_stmt(codegen_ctx_t *ctx, global_stmt_t *gstmt) {
  switch (gstmt->type) {
  case GSTMT_FUNC_DECL: {
    type_t *ret_type = gstmt->value.func.ret_type;
    ret_type = complete_type(ctx, ret_type);
    add_function(ctx, ret_type, gstmt->value.func.name);
    break;
  }
  case GSTMT_FUNC:
    init_ctx(ctx, gstmt->value.func.name);
    push_scope(ctx);

    type_t *ret_type = gstmt->value.func.ret_type;
    ret_type = complete_type(ctx, ret_type);
    add_function(ctx, ret_type, gstmt->value.func.name);

    gen(ctx, ".global %s\n", ctx->cur_func_name);
    gen(ctx, "%s:\n", ctx->cur_func_name);
    gen(ctx, ".loc 1 %d %d\n", gstmt->pos->line, gstmt->pos->column);
    gen(ctx, "  stp x29, x30, [sp, -0x100]!\n"); // TODO
    gen(ctx, "  mov x29, sp\n");

    gen_func_parameter(ctx, gstmt->value.func.params, gstmt->pos);
    gen_stmt(ctx, gstmt->value.func.body);

    gen(ctx, ".L.%s.ret:\n", ctx->cur_func_name);
    gen_pop(ctx, "x0");
    gen(ctx, "  mov sp, x29\n");
    gen(ctx, "  ldp x29, x30, [sp], 0x100\n");
    gen(ctx, "  ret\n");

    pop_scope(ctx);
    break;
  case GSTMT_STRUCT:
  case GSTMT_UNION:
    add_type(ctx, gstmt->value.type);
    break;
  case GSTMT_ENUM:
    add_type(ctx, gstmt->value.type);
    append_enums(ctx, gstmt->value.type->value.enum_.enums);
    break;
  case GSTMT_TYPEDEF:
    if (gstmt->value.type->kind == TYPE_ENUM) {
      add_type(ctx, gstmt->value.type);
      append_enums(ctx, gstmt->value.type->value.enum_.enums);
    }
    break;
  case GSTMT_DEFINE:
    // do nothing
    break;
  }
}

void gen_text(codegen_ctx_t *ctx, global_stmt_t *gstmt) {
  gen(ctx, ".text\n");
  gen(ctx, ".file 1 \"%s\"\n", ctx->in_filepath);

  global_stmt_t *cur = gstmt;
  while (cur) {
    gen_global_stmt(ctx, cur);
    cur = cur->next;
  }
}

void gen_string(codegen_ctx_t *ctx, char *string) {
  gen(ctx, "  .string \"");

  char *p = string;
  char buf[2];
  buf[1] = 0;

  while (*p) {
    switch (*p) {
    case '\n':
      gen(ctx, "\\n");
      break;
    case '\\':
      gen(ctx, "\\\\");
      break;
    case '\'':
      gen(ctx, "\\'");
      break;
    case '\"':
      gen(ctx, "\\\"");
      break;
    case '%':
      gen(ctx, "%%");
      break;
    default:
      buf[0] = *p;
      gen(ctx, buf);
      break;
    }

    p++;
  }

  gen(ctx, "\\0\"\n");
}

void gen_strings(codegen_ctx_t *ctx) {
  string_t *cur = ctx->strings;
  int str_index = ctx->cur_string;

  while (cur) {
    gen(ctx, ".L.str.%d:\n", str_index);
    gen_string(ctx, cur->string);

    cur = cur->next;
    str_index--;
  }
}

void gen_globals(codegen_ctx_t *ctx) {
  global_var_t *cur = ctx->globals;
  while (cur) {
    if (cur->type->is_extern) {
      cur = cur->next;
      continue;
    }

    int size = type_size(cur->type);
    gen(ctx, ".global %s\n", cur->name);
    gen(ctx, "%s:\n", cur->name);
    gen(ctx, "  .zero %d\n", size);

    cur = cur->next;
  }
}

void gen_data(codegen_ctx_t *ctx) {
  gen(ctx, ".data\n");

  gen_strings(ctx);
  gen_globals(ctx);
}

void gen_code(program_t *program, char *in_filepath, FILE *out_fp) {
  codegen_ctx_t *ctx = new_codegen_ctx(in_filepath, out_fp, program->globals);

  init_arg_regs();
  gen_text(ctx, program->body);
  gen_data(ctx);
}
