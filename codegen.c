#include "codegen.h"
#include "error.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

char *arg_regs[] = {"x0", "x1", "x2", "x3", "x4", "x5", "x6"};

void gen_expr(codegen_ctx_t *ctx, expr_t *expr);
void gen_stmt(codegen_ctx_t *ctx, stmt_t *stmt);

codegen_ctx_t *new_codegen_ctx(FILE *fp) {
  codegen_ctx_t *ctx = calloc(1, sizeof(codegen_ctx_t));
  ctx->fp = fp;
  ctx->cur_offset = 16;
  return ctx;
}

void init_ctx(codegen_ctx_t *ctx, char *func_name) {
  ctx->variables = NULL;
  ctx->cur_offset = 16;
  ctx->cur_label = 0;
  ctx->cur_func_name = func_name;
}

variable_t *add_variable(codegen_ctx_t *ctx, type_t *type, char *name) {
  variable_t *variable = calloc(1, sizeof(variable_t));
  variable->type = type;
  variable->name = name;

  int offset = align_to(ctx->cur_offset + type_size(type), type_align(type));
  ctx->cur_offset = offset;
  variable->offset = offset;

  variable->next = ctx->variables;
  ctx->variables = variable;

  return variable;
}

variable_t *find_variable(codegen_ctx_t *ctx, char *name) {
  variable_t *cur = ctx->variables;
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

void gen(codegen_ctx_t *ctx, char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(ctx->fp, format, args);
  va_end(args);
}

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

void gen_load(codegen_ctx_t *ctx, type_t *type) {
  gen_pop(ctx, "x8");
  switch (type_size(type)) {
  case 4:
    gen(ctx, "  ldr w8, [x8]\n");
    break;
  case 8:
    gen(ctx, "  ldr x8, [x8]\n");
    break;
  default:
    panic("cannot load: type=%d\n", type->kind);
  }
  gen_push(ctx, "x8");
}

void gen_store(codegen_ctx_t *ctx, type_t *type) {
  gen_pop(ctx, "x8"); // dst
  gen_pop(ctx, "x9"); // src
  switch (type_size(type)) {
  case 4:
    gen(ctx, "  str w9, [x8]\n");
    break;
  case 8:
    gen(ctx, "  str x9, [x8]\n");
    break;
  default:
    panic("cannot store: type=%d\n", type->kind);
  }
}

void gen_var_addr(codegen_ctx_t *ctx, variable_t *var) {
  gen(ctx, "  add x8, x29, %d\n", var->offset);
  gen_push(ctx, "x8");
}

void gen_lvalue(codegen_ctx_t *ctx, expr_t *expr) {
  switch (expr->type) {
  case EXPR_IDENT: {
    variable_t *var = find_variable(ctx, expr->value.ident);
    if (var == NULL) {
      panic("unknown variable '%s'\n", expr->value.ident);
    }
    gen_var_addr(ctx, var);
    break;
  case EXPR_DEREF:
    gen_expr(ctx, expr->value.unary);
    break;
  }
  default:
    panic("cannot generate lvalue: expr=%d\n", expr->type);
  }
}

type_t *infer_expr_type(codegen_ctx_t *ctx, expr_t *expr) {
  switch (expr->type) {
  case EXPR_NUMBER:
    return new_type(TYPE_INT);
  case EXPR_IDENT: {
    variable_t *var = find_variable(ctx, expr->value.ident);
    if (var == NULL) {
      panic("unknown variable '%s'\n", expr->value.ident);
    }

    return var->type;
  }
  case EXPR_ADD: {
    type_t *lhs_type = infer_expr_type(ctx, expr->value.binary.lhs);
    type_t *rhs_type = infer_expr_type(ctx, expr->value.binary.rhs);
    if (lhs_type->kind == TYPE_INT && rhs_type->kind == TYPE_INT) {
      return lhs_type;
    } else if (is_ptr(lhs_type) && rhs_type->kind == TYPE_INT) {
      return lhs_type;
    } else if (lhs_type->kind == TYPE_INT && is_ptr(rhs_type)) {
      return rhs_type;
    }
    panic("invalid add operation: lhs=%d, rhs=%d\n", lhs_type->kind,
          rhs_type->kind);
  }
  case EXPR_SUB: {
    type_t *lhs_type = infer_expr_type(ctx, expr->value.binary.lhs);
    type_t *rhs_type = infer_expr_type(ctx, expr->value.binary.rhs);
    if (lhs_type->kind == TYPE_INT && rhs_type->kind == TYPE_INT) {
      return lhs_type;
    } else if (is_ptr(lhs_type) && rhs_type->kind == TYPE_INT) {
      return lhs_type;
    } else if (is_ptr(lhs_type) && is_ptr(rhs_type)) {
      return new_type(TYPE_INT);
    }
    panic("invalid sub operation: lhs=%d, rhs=%d\n", lhs_type->kind,
          rhs_type->kind);
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
    return new_type(TYPE_INT); // TODO
  case EXPR_ASSIGN:
    return infer_expr_type(ctx, expr->value.assign.dst);
  case EXPR_CALL:
    return new_type(TYPE_INT); // TODO
  case EXPR_REF:
    return ptr_to(infer_expr_type(ctx, expr->value.unary));
  case EXPR_DEREF: {
    type_t *ptr_type = infer_expr_type(ctx, expr->value.unary);
    return type_deref(ptr_type);
  }
  case EXPR_SIZEOF:
    return new_type(TYPE_INT);
  }
  panic("unreachable\n");
}

void gen_expr(codegen_ctx_t *ctx, expr_t *expr) {
  switch (expr->type) {
  case EXPR_NUMBER:
    gen(ctx, "  mov x8, %d\n", expr->value.number);
    gen_push(ctx, "x8");
    return;
  case EXPR_IDENT: {
    variable_t *var = find_variable(ctx, expr->value.ident);
    if (var == NULL) {
      panic("unknown variable '%s'\n", expr->value.ident);
    }
    gen_var_addr(ctx, var);
    if (var->type->kind != TYPE_ARRAY) {
      gen_load(ctx, var->type);
    }
    return;
  }
  case EXPR_ASSIGN:
    gen_expr(ctx, expr->value.assign.src);
    gen_lvalue(ctx, expr->value.assign.dst);
    gen_store(ctx, infer_expr_type(ctx, expr));
    gen_push(ctx, "x8"); // FIXME
    return;
  case EXPR_CALL: {
    int i = 0;
    argument_t *cur_arg = expr->value.call.args;
    while (cur_arg) {
      if (i > 7) {
        panic("cannot use > 7 arguments\n");
      }

      gen_expr(ctx, cur_arg->value);
      gen_pop(ctx, arg_regs[i]);

      cur_arg = cur_arg->next;
      i++;
    }

    gen(ctx, "  bl %s\n", expr->value.ident);
    gen_push(ctx, "x0");
    return;
  }
  default:
    break;
  }

  // unary op
  switch (expr->type) {
  case EXPR_REF:
    gen_lvalue(ctx, expr->value.unary);
    return;
  case EXPR_DEREF:
    gen_expr(ctx, expr->value.unary);
    gen_load(ctx, infer_expr_type(ctx, expr));
    return;
  case EXPR_SIZEOF: {
    type_t *type = infer_expr_type(ctx, expr->value.unary);
    gen(ctx, "  mov x8, %d\n", type_size(type));
    gen_push(ctx, "x8");
    return;
  }
  default:
    break;
  }

  // binary op
  gen_expr(ctx, expr->value.binary.lhs);
  gen_expr(ctx, expr->value.binary.rhs);
  gen_pop(ctx, "x9");
  gen_pop(ctx, "x8");

  switch (expr->type) {
  case EXPR_ADD: {
    type_t *lhs_type = infer_expr_type(ctx, expr->value.binary.lhs);
    type_t *rhs_type = infer_expr_type(ctx, expr->value.binary.rhs);
    if (lhs_type->kind == TYPE_INT && rhs_type->kind == TYPE_INT) {
      // do nothing
    } else if (is_ptr(lhs_type) && rhs_type->kind == TYPE_INT) {
      gen(ctx, "  mov x10, %d\n", type_size(type_deref(lhs_type)));
      gen(ctx, "  mul x9, x9, x10\n");
    } else if (lhs_type->kind == TYPE_INT && is_ptr(rhs_type)) {
      gen(ctx, "  mov x10, %d\n", type_size(type_deref(rhs_type)));
      gen(ctx, "  mul x8, x8, x10\n");
    } else {
      panic("invalid add operation: lhs=%d, rhs=%d\n", lhs_type->kind,
            rhs_type->kind);
    }
    gen(ctx, "  add x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  }
  case EXPR_SUB: {
    type_t *lhs_type = infer_expr_type(ctx, expr->value.binary.lhs);
    type_t *rhs_type = infer_expr_type(ctx, expr->value.binary.rhs);
    if (lhs_type->kind == TYPE_INT && rhs_type->kind == TYPE_INT) {
      gen(ctx, "  sub x8, x8, x9\n");
    } else if (is_ptr(lhs_type) && rhs_type->kind == TYPE_INT) {
      gen(ctx, "  mov x10, %d\n", type_size(type_deref(lhs_type)));
      gen(ctx, "  mul x9, x9, x10\n");
      gen(ctx, "  sub x8, x8, x9\n");
    } else if (is_ptr(lhs_type) && is_ptr(rhs_type)) {
      gen(ctx, "  sub x8, x8, x9\n");
      gen(ctx, "  mov x9, %d\n", type_size(type_deref(lhs_type)));
      gen(ctx, "  udiv x8, x8, x9\n");
    } else {
      panic("invalid add operation: lhs=%d, rhs=%d\n", lhs_type->kind,
            rhs_type->kind);
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
  default:
    break;
  }
}

void gen_stmt(codegen_ctx_t *ctx, stmt_t *stmt) {
  switch (stmt->type) {
  case STMT_EXPR:
    gen_expr(ctx, stmt->value.expr);
    break;
  case STMT_RETURN:
    gen_expr(ctx, stmt->value.ret);
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
    gen_label(ctx, cond_label);
    gen_expr(ctx, stmt->value.while_.cond);
    gen(ctx, "  subs x8, x8, 0\n");
    gen_branch(ctx, "beq", end_label);

    gen_stmt(ctx, stmt->value.while_.body);
    gen_branch(ctx, "b", cond_label);

    gen_label(ctx, end_label);
    break;
  }
  case STMT_FOR: {
    int cond_label = next_label(ctx);
    int end_label = next_label(ctx);
    if (stmt->value.for_.init) {
      gen_stmt(ctx, stmt->value.for_.init);
    }

    gen_label(ctx, cond_label);
    if (stmt->value.for_.cond) {
      gen_expr(ctx, stmt->value.for_.cond);
      gen(ctx, "  subs x8, x8, 0\n");
      gen_branch(ctx, "beq", end_label);
    }

    gen_stmt(ctx, stmt->value.for_.body);
    if (stmt->value.for_.loop) {
      gen_expr(ctx, stmt->value.for_.loop);
    }
    gen_branch(ctx, "b", cond_label);

    gen_label(ctx, end_label);
    break;
  }
  case STMT_BLOCK: {
    stmt_list_t *cur = stmt->value.block;
    while (cur) {
      gen_stmt(ctx, cur->stmt);
      cur = cur->next;
    }
    break;
  }
  case STMT_DEFINE: {
    char *name = stmt->value.define.name;
    if (find_variable(ctx, name) != NULL) {
      panic("variable '%s' already defined\n", name);
    }
    variable_t *var = add_variable(ctx, stmt->value.define.type, name);
    if (stmt->value.define.value) {
      gen_expr(ctx, stmt->value.define.value);
      gen_var_addr(ctx, var);
      gen_store(ctx, var->type);
    }
    break;
  }
  }
}

void gen_func_parameter(codegen_ctx_t *ctx, parameter_t *params) {
  int i = 0;
  while (params) {
    if (i > 7) {
      panic("cannot use > 7 arguments\n");
    }

    variable_t *var = add_variable(ctx, params->type, params->name);
    gen_push(ctx, arg_regs[i]);
    gen_var_addr(ctx, var);
    gen_store(ctx, var->type);

    params = params->next;
    i++;
  }
}

void gen_global_stmt(codegen_ctx_t *ctx, global_stmt_t *gstmt) {
  switch (gstmt->type) {
  case GSTMT_FUNC:
    init_ctx(ctx, gstmt->value.func.name);

    gen(ctx, ".global %s\n", ctx->cur_func_name);
    gen(ctx, "%s:\n", ctx->cur_func_name);
    gen(ctx, "  stp x29, x30, [sp, -0x100]!\n"); // TODO
    gen(ctx, "  mov x29, sp\n");

    gen_func_parameter(ctx, gstmt->value.func.params);
    gen_stmt(ctx, gstmt->value.func.body);

    gen(ctx, ".L.%s.ret:\n", ctx->cur_func_name);
    gen_pop(ctx, "x0");
    gen(ctx, "  mov sp, x29\n");
    gen(ctx, "  ldp x29, x30, [sp], 0x100\n");
    gen(ctx, "  ret\n");
  }
}

void gen_code(global_stmt_t *gstmt, FILE *fp) {
  codegen_ctx_t *ctx = new_codegen_ctx(fp);

  global_stmt_t *cur = gstmt;
  while (cur) {
    gen_global_stmt(ctx, cur);
    cur = cur->next;
  }
}
