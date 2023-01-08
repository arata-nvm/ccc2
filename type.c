#include "type.h"
#include "error.h"
#include <stdlib.h>

type_t *new_type(typekind_t kind) {
  type_t *type = calloc(1, sizeof(type_t));
  type->kind = kind;
  return type;
}

type_t *ptr_to(type_t *base_type) {
  type_t *type = new_type(TYPE_PTR);
  type->value.ptr = base_type;
  return type;
}

type_t *array_of(type_t *elm_type, int len) {
  type_t *type = new_type(TYPE_ARRAY);
  type->value.array.elm = elm_type;
  type->value.array.len = len;
  return type;
}

int type_size(type_t *type) {
  switch (type->kind) {
  case TYPE_INT:
    return 4;
  case TYPE_PTR:
    return 8;
  case TYPE_ARRAY:
    return type_size(type->value.array.elm) * type->value.array.len;
  default:
    panic("unknown type: type=%d\n", type->kind);
  }
}

int type_align(type_t *type) {
  switch (type->kind) {
  case TYPE_INT:
    return 4;
  case TYPE_PTR:
  case TYPE_ARRAY:
    return 8;
  default:
    panic("unknown type: type=%d\n", type->kind);
  }
}

type_t *type_deref(type_t *type) {
  switch (type->kind) {
  case TYPE_PTR:
    return type->value.ptr;
  case TYPE_ARRAY:
    return type->value.array.elm;
  default:
    panic("cannot dereference: type=%d\n", type->kind);
  }
}

int is_ptr(type_t *type) {
  return type->kind == TYPE_PTR || type->kind == TYPE_ARRAY;
}

int align_to(int n, int align) { return (n + align - 1) & ~(align - 1); }
