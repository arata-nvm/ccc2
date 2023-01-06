#include "codegen.h"

void push(FILE *fp, char *reg) {
  fprintf(fp, "  sub sp, sp, 16\n");
  fprintf(fp, "  str %s, [sp, 16]\n", reg);
}

void pop(FILE *fp, char *reg) {
  fprintf(fp, "  ldr %s, [sp, 16]\n", reg);
  fprintf(fp, "  add sp, sp, 16\n");
}

void gen_node(node_t *node, FILE *fp) {
  switch (node->type) {
  case NODE_NUMBER:
    fprintf(fp, "  mov x0, %d\n", node->value.number);
    push(fp, "x0");
    return;
  }

  // binary op
  gen_node(node->value.binary.lhs, fp);
  gen_node(node->value.binary.rhs, fp);
  pop(fp, "x1");
  pop(fp, "x0");

  switch (node->type) {
  case NODE_ADD:
    fprintf(fp, "  add x0, x0, x1\n");
    push(fp, "x0");
    break;
  case NODE_SUB:
    fprintf(fp, "  sub x0, x0, x1\n");
    push(fp, "x0");
    break;
  }
}

void gen_code(node_t *node, FILE *fp) {
  fprintf(fp, ".global main\n");
  fprintf(fp, "main:\n");
  gen_node(node, fp);
  pop(fp, "x0");
  fprintf(fp, "  ret\n");
}
