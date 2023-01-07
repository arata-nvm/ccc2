#include "codegen.h"
#include "error.h"
#include <stdarg.h>
#include <string.h>

void gen_expr(expr_t *expr, codegen_ctx_t *ctx);
void gen_stmt(stmt_t *stmt, codegen_ctx_t *ctx);

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

void add_variable(codegen_ctx_t *ctx, char *name, int offset) {
  variable_t *variable = calloc(1, sizeof(variable_t));
  variable->name = name;
  variable->offset = offset;
  variable->next = ctx->variables;
  ctx->variables = variable;
}

int find_variable(codegen_ctx_t *ctx, char *name) {
  variable_t *cur = ctx->variables;
  while (cur) {
    if (!strcmp(cur->name, name)) {
      return cur->offset;
    }
    cur = cur->next;
  }

  return -1;
}

int next_offset(codegen_ctx_t *ctx) {
  ctx->cur_offset += 16;
  return ctx->cur_offset;
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

void gen_push(codegen_ctx_t *ctx, char *reg) {
  gen(ctx, "  sub sp, sp, 16\n");
  gen(ctx, "  str %s, [sp, 16]\n", reg);
}

void gen_pop(codegen_ctx_t *ctx, char *reg) {
  gen(ctx, "  ldr %s, [sp, 16]\n", reg);
  gen(ctx, "  add sp, sp, 16\n");
}

void gen_lvalue(expr_t *expr, codegen_ctx_t *ctx) {
  switch (expr->type) {
  case EXPR_IDENT: {
    char *name = expr->value.ident;
    int offset = find_variable(ctx, name);
    if (offset == -1) { // TODO
      offset = next_offset(ctx);
      add_variable(ctx, name, ctx->cur_offset);
    }
    gen(ctx, "  add x8, x29, %d\n", offset);
    gen_push(ctx, "x8");
    break;
  case EXPR_DEREF:
    gen_expr(expr->value.unary, ctx);
    break;
  }
  default:
    error("cannot generate lvalue");
  }
}

void gen_expr(expr_t *expr, codegen_ctx_t *ctx) {
  switch (expr->type) {
  case EXPR_NUMBER:
    gen(ctx, "  mov x8, %d\n", expr->value.number);
    gen_push(ctx, "x8");
    return;
  case EXPR_IDENT: {
    int offset = find_variable(ctx, expr->value.ident);
    if (offset == -1) {
      error("unknown variable");
    }
    gen(ctx, "  ldr x8, [x29, %d]\n", offset);
    gen_push(ctx, "x8");
    return;
  }
  case EXPR_ASSIGN:
    gen_expr(expr->value.assign.src, ctx);
    gen_lvalue(expr->value.assign.dst, ctx);
    gen_pop(ctx, "x9");
    gen_pop(ctx, "x8");
    gen(ctx, "  str x8, [x9]\n");
    gen_push(ctx, "x8");
    return;
  case EXPR_CALL: {
    int i = 0;
    char reg_name[3];
    argument_t *cur_arg = expr->value.call.args;
    while (cur_arg) {
      gen_expr(cur_arg->value, ctx);
      if (i > 7) {
        error("cannot use > 7 arguments");
      }
      snprintf(reg_name, 3, "x%d", i);
      gen_pop(ctx, reg_name);
      i++;
      cur_arg = cur_arg->next;
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
    gen_lvalue(expr->value.unary, ctx);
    return;
  case EXPR_DEREF:
    gen_expr(expr->value.unary, ctx);
    gen_pop(ctx, "x8");
    gen(ctx, "  ldr x8, [x8]\n");
    gen_push(ctx, "x8");
    return;
  default:
    break;
  }

  // binary op
  gen_expr(expr->value.binary.lhs, ctx);
  gen_expr(expr->value.binary.rhs, ctx);
  gen_pop(ctx, "x9");
  gen_pop(ctx, "x8");

  switch (expr->type) {
  case EXPR_ADD:
    gen(ctx, "  add x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
  case EXPR_SUB:
    gen(ctx, "  sub x8, x8, x9\n");
    gen_push(ctx, "x8");
    break;
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

void gen_stmt(stmt_t *stmt, codegen_ctx_t *ctx) {
  switch (stmt->type) {
  case STMT_EXPR:
    gen_expr(stmt->value.expr, ctx);
    break;
  case STMT_RETURN:
    gen_expr(stmt->value.ret, ctx);
    gen(ctx, "  b .L%s.ret\n", ctx->cur_func_name);
    break;
  case STMT_IF: {
    int else_label = next_label(ctx);
    int merge_label = next_label(ctx);
    gen_expr(stmt->value.if_.cond, ctx);
    gen_pop(ctx, "x8");
    gen(ctx, "  subs x8, x8, 0\n");
    gen(ctx, "  beq .L%s.if.%d\n", ctx->cur_func_name, else_label);
    gen_stmt(stmt->value.if_.then_, ctx);
    if (stmt->value.if_.else_) {
      gen(ctx, "b .L%s.if.%d\n", ctx->cur_func_name, merge_label);
    }
    gen(ctx, ".L%s.if.%d:\n", ctx->cur_func_name, else_label);
    if (stmt->value.if_.else_) {
      gen_stmt(stmt->value.if_.else_, ctx);
      gen(ctx, ".L%s.if.%d:\n", ctx->cur_func_name, merge_label);
    }
    break;
  }
  case STMT_WHILE: {
    int cond_label = next_label(ctx);
    int end_label = next_label(ctx);
    gen(ctx, ".L%s.while.%d:\n", ctx->cur_func_name, cond_label);
    gen_expr(stmt->value.while_.cond, ctx);
    gen(ctx, "  subs x8, x8, 0\n");
    gen(ctx, "  beq .L%s.while.%d\n", ctx->cur_func_name, end_label);
    gen_stmt(stmt->value.while_.body, ctx);
    gen(ctx, "  b .L%s.while.%d\n", ctx->cur_func_name, cond_label);
    gen(ctx, ".L%s.while.%d:\n", ctx->cur_func_name, end_label);
    break;
  }
  case STMT_FOR: {
    int cond_label = next_label(ctx);
    int end_label = next_label(ctx);
    if (stmt->value.for_.init) {
      gen_stmt(stmt->value.for_.init, ctx);
    }
    gen(ctx, ".L%s.for.%d:\n", ctx->cur_func_name, cond_label);
    if (stmt->value.for_.cond) {
      gen_expr(stmt->value.for_.cond, ctx);
      gen(ctx, "  subs x8, x8, 0\n");
      gen(ctx, "  beq .L%s.for.%d\n", ctx->cur_func_name, end_label);
    }
    gen_stmt(stmt->value.for_.body, ctx);
    if (stmt->value.for_.loop) {
      gen_expr(stmt->value.for_.loop, ctx);
    }
    gen(ctx, "  b .L%s.for.%d\n", ctx->cur_func_name, cond_label);
    gen(ctx, ".L%s.for.%d:\n", ctx->cur_func_name, end_label);
    break;
  }
  case STMT_BLOCK: {
    stmt_list_t *cur = stmt->value.block;
    while (cur) {
      gen_stmt(cur->stmt, ctx);
      cur = cur->next;
    }
    break;
  }
  }
}

void gen_func_parameter(parameter_t *params, codegen_ctx_t *ctx) {
  int i = 0;
  char reg_name[3];
  while (params) {
    int offset = next_offset(ctx);
    add_variable(ctx, params->name, offset);
    if (i > 7) {
      error("cannot use > 7 arguments");
    }
    snprintf(reg_name, 3, "x%d", i);
    gen(ctx, "  str %s, [sp, %d]\n", reg_name, offset);
    i++;
    params = params->next;
  }
}

void gen_global_stmt(global_stmt_t *gstmt, codegen_ctx_t *ctx) {
  switch (gstmt->type) {
  case GSTMT_FUNC:
    init_ctx(ctx, gstmt->value.func.name);

    gen(ctx, ".global %s\n", ctx->cur_func_name);
    gen(ctx, "%s:\n", ctx->cur_func_name);
    gen(ctx, "  sub sp, sp, 0x900\n"); // TODO
    gen(ctx, "  stp x29, x30, [sp, 16]\n");
    gen(ctx, "  mov x29, sp\n");

    gen_func_parameter(gstmt->value.func.params, ctx);
    gen_stmt(gstmt->value.func.body, ctx);

    gen(ctx, ".L%s.ret:\n", ctx->cur_func_name);
    gen_pop(ctx, "x0");
    gen(ctx, "  mov sp, x29\n");
    gen(ctx, "  ldp x29, x30, [sp, 16]\n");
    gen(ctx, "  add sp, sp, 0x900\n");
    gen(ctx, "  ret\n");
  }
}

void gen_code(global_stmt_t *gstmt, FILE *fp) {
  codegen_ctx_t *ctx = new_codegen_ctx(fp);

  global_stmt_t *cur = gstmt;
  while (cur) {
    gen_global_stmt(cur, ctx);
    cur = cur->next;
  }
}
