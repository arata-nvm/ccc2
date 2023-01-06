#include "codegen.h"
#include "error.h"

codegen_ctx_t *new_codegen_ctx(FILE *fp) {
  codegen_ctx_t *ctx = calloc(1, sizeof(codegen_ctx_t));
  ctx->fp = fp;
  return ctx;
}

inline void gen(codegen_ctx_t *ctx, char *format, ...) {
  fprintf(ctx->fp, format, __va_arg_pack());
}

void gen_push(codegen_ctx_t *ctx, char *reg) {
  gen(ctx, "  sub sp, sp, 16\n");
  gen(ctx, "  str %s, [sp, 16]\n", reg);
}

void gen_pop(codegen_ctx_t *ctx, char *reg) {
  gen(ctx, "  ldr %s, [sp, 16]\n", reg);
  gen(ctx, "  add sp, sp, 16\n");
}

void gen_expr(expr_t *expr, codegen_ctx_t *ctx) {
  switch (expr->type) {
  case EXPR_NUMBER:
    gen(ctx, "  mov x0, %d\n", expr->value.number);
    gen_push(ctx, "x0");
    return;
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
  }
}

void gen_stmt(stmt_t *stmt, codegen_ctx_t *ctx) {
  switch (stmt->type) {
  case STMT_EXPR:
    gen_expr(stmt->value.expr, ctx);
    break;
  }
}

void gen_code(stmt_t *stmt, FILE *fp) {
  codegen_ctx_t *ctx = new_codegen_ctx(fp);

  gen(ctx, ".global main\n");
  gen(ctx, "main:\n");

  stmt_t *cur = stmt;
  while (cur) {
    gen_stmt(cur, fp);
    cur = cur->next;
  }

  gen_pop(ctx, "x0");
  gen(ctx, "  ret\n");
}
