#include "codegen.h"

void push(FILE *fp, char *reg) {
  fprintf(fp, "  sub sp, sp, 16\n");
  fprintf(fp, "  str %s, [sp, 16]\n", reg);
}

void pop(FILE *fp, char *reg) {
  fprintf(fp, "  ldr %s, [sp, 16]\n", reg);
  fprintf(fp, "  add sp, sp, 16\n");
}

void gen_expr(expr_t *expr, FILE *fp) {
  switch (expr->type) {
  case EXPR_NUMBER:
    fprintf(fp, "  mov x0, %d\n", expr->value.number);
    push(fp, "x0");
    return;
  }

  // binary op
  gen_expr(expr->value.binary.lhs, fp);
  gen_expr(expr->value.binary.rhs, fp);
  pop(fp, "x1");
  pop(fp, "x0");

  switch (expr->type) {
  case EXPR_ADD:
    fprintf(fp, "  add x0, x0, x1\n");
    push(fp, "x0");
    break;
  case EXPR_SUB:
    fprintf(fp, "  sub x0, x0, x1\n");
    push(fp, "x0");
    break;
  case EXPR_MUL:
    fprintf(fp, "  mul x0, x0, x1\n");
    push(fp, "x0");
    break;
  case EXPR_DIV:
    fprintf(fp, "  sdiv x0, x0, x1\n");
    push(fp, "x0");
    break;
  case EXPR_REM:
    fprintf(fp, "  sdiv x2, x0, x1\n");
    fprintf(fp, "  msub x0, x1, x2, x0\n");
    push(fp, "x0");
    break;
  case EXPR_LT:
    fprintf(fp, "  subs x0, x0, x1\n");
    fprintf(fp, "  cset x0, lt\n");
    push(fp, "x0");
    break;
  case EXPR_LE:
    fprintf(fp, "  subs x0, x0, x1\n");
    fprintf(fp, "  cset x0, le\n");
    push(fp, "x0");
    break;
  case EXPR_GT:
    fprintf(fp, "  subs x0, x0, x1\n");
    fprintf(fp, "  cset x0, gt\n");
    push(fp, "x0");
    break;
  case EXPR_GE:
    fprintf(fp, "  subs x0, x0, x1\n");
    fprintf(fp, "  cset x0, ge\n");
    push(fp, "x0");
    break;
  case EXPR_EQ:
    fprintf(fp, "  subs x0, x0, x1\n");
    fprintf(fp, "  cset x0, eq\n");
    push(fp, "x0");
    break;
  case EXPR_NE:
    fprintf(fp, "  subs x0, x0, x1\n");
    fprintf(fp, "  cset x0, ne\n");
    push(fp, "x0");
    break;
  }
}

void gen_stmt(stmt_t *stmt, FILE *fp) {
  switch (stmt->type) {
  case STMT_EXPR:
    gen_expr(stmt->value.expr, fp);
    break;
  }
}

void gen_code(stmt_t *stmt, FILE *fp) {
  fprintf(fp, ".global main\n");
  fprintf(fp, "main:\n");

  stmt_t *cur = stmt;
  while (cur) {
    gen_stmt(cur, fp);
    cur = cur->next;
  }

  pop(fp, "x0");
  fprintf(fp, "  ret\n");
}
