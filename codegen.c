#include "codegen.h"
#include "error.h"
#include <stdarg.h>
#include <string.h>

codegen_ctx_t *new_codegen_ctx(FILE *fp) {
  codegen_ctx_t *ctx = calloc(1, sizeof(codegen_ctx_t));
  ctx->fp = fp;
  return ctx;
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
    gen(ctx, "  add x0, x29, %d\n", offset);
    gen_push(ctx, "x0");
    break;
  }
  default:
    error("cannot generate lvalue");
  }
}

void gen_expr(expr_t *expr, codegen_ctx_t *ctx) {
  switch (expr->type) {
  case EXPR_NUMBER:
    gen(ctx, "  mov x0, %d\n", expr->value.number);
    gen_push(ctx, "x0");
    return;
  case EXPR_IDENT: {
    int offset = find_variable(ctx, expr->value.ident);
    if (offset == -1) {
      error("unknown variable");
    }
    gen(ctx, "  ldr x0, [x29, %d]\n", offset);
    gen_push(ctx, "x0");
    return;
  }
  case EXPR_ASSIGN:
    gen_expr(expr->value.assign.src, ctx);
    gen_lvalue(expr->value.assign.dst, ctx);
    gen_pop(ctx, "x1");
    gen_pop(ctx, "x0");
    gen(ctx, "  str x0, [x1]\n");
    gen_push(ctx, "x0");
    return;
  default:
    break;
  }

  // binary op
  gen_expr(expr->value.binary.lhs, ctx);
  gen_expr(expr->value.binary.rhs, ctx);
  gen_pop(ctx, "x1");
  gen_pop(ctx, "x0");

  switch (expr->type) {
  case EXPR_ADD:
    gen(ctx, "  add x0, x0, x1\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_SUB:
    gen(ctx, "  sub x0, x0, x1\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_MUL:
    gen(ctx, "  mul x0, x0, x1\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_DIV:
    gen(ctx, "  sdiv x0, x0, x1\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_REM:
    gen(ctx, "  sdiv x2, x0, x1\n");
    gen(ctx, "  msub x0, x1, x2, x0\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_LT:
    gen(ctx, "  subs x0, x0, x1\n");
    gen(ctx, "  cset x0, lt\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_LE:
    gen(ctx, "  subs x0, x0, x1\n");
    gen(ctx, "  cset x0, le\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_GT:
    gen(ctx, "  subs x0, x0, x1\n");
    gen(ctx, "  cset x0, gt\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_GE:
    gen(ctx, "  subs x0, x0, x1\n");
    gen(ctx, "  cset x0, ge\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_EQ:
    gen(ctx, "  subs x0, x0, x1\n");
    gen(ctx, "  cset x0, eq\n");
    gen_push(ctx, "x0");
    break;
  case EXPR_NE:
    gen(ctx, "  subs x0, x0, x1\n");
    gen(ctx, "  cset x0, ne\n");
    gen_push(ctx, "x0");
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
    gen(ctx, "  b main.ret\n");
    break;
  case STMT_IF: {
    int else_label = next_label(ctx);
    int merge_label = next_label(ctx);
    gen_expr(stmt->value.if_.cond, ctx);
    gen_pop(ctx, "x0");
    gen(ctx, "  subs x0, x0, 0\n");
    gen(ctx, "  beq .if.%d\n", else_label);
    gen_stmt(stmt->value.if_.then_, ctx);
    if (stmt->value.if_.else_) {
      gen(ctx, "b .if.%d\n", merge_label);
    }
    gen(ctx, ".if.%d:\n", else_label);
    if (stmt->value.if_.else_) {
      gen_stmt(stmt->value.if_.else_, ctx);
      gen(ctx, ".if.%d:\n", merge_label);
    }
    break;
  }
  case STMT_WHILE: {
    int cond_label = next_label(ctx);
    int end_label = next_label(ctx);
    gen(ctx, ".while.%d:\n", cond_label);
    gen_expr(stmt->value.while_.cond, ctx);
    gen(ctx, "  subs x0, x0, 0\n");
    gen(ctx, "  beq .while.%d\n", end_label);
    gen_stmt(stmt->value.while_.body, ctx);
    gen(ctx, "  b .while.%d\n", cond_label);
    gen(ctx, ".while.%d:\n", end_label);
    break;
  }
  case STMT_FOR: {
    int cond_label = next_label(ctx);
    int end_label = next_label(ctx);
    if (stmt->value.for_.init) {
      gen_expr(stmt->value.for_.init, ctx);
    }
    gen(ctx, ".for.%d:\n", cond_label);
    if (stmt->value.for_.cond) {
      gen_expr(stmt->value.for_.cond, ctx);
      gen(ctx, "  subs x0, x0, 0\n");
      gen(ctx, "  beq .for.%d\n", end_label);
    }
    gen_stmt(stmt->value.for_.body, ctx);
    if (stmt->value.for_.loop) {
      gen_expr(stmt->value.for_.loop, ctx);
    }
    gen(ctx, "  b .for.%d\n", cond_label);
    gen(ctx, ".for.%d:\n", end_label);
  }
  }
}

void gen_code(stmt_t *stmt, FILE *fp) {
  codegen_ctx_t *ctx = new_codegen_ctx(fp);

  gen(ctx, ".global main\n");
  gen(ctx, "main:\n");
  gen(ctx, "  sub sp, sp, 0x100\n"); // TODO
  gen(ctx, "  mov x29, sp\n");

  stmt_t *cur = stmt;
  while (cur) {
    gen_stmt(cur, ctx);
    cur = cur->next;
  }

  gen(ctx, "main.ret:\n");
  gen_pop(ctx, "x0");
  gen(ctx, "  mov sp, x29\n");
  gen(ctx, "  add sp, sp, 0x100\n");
  gen(ctx, "  ret\n");
}
