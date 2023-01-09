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

struct_member_t *new_struct_member(type_t *type, char *name) {
  struct_member_t *member = calloc(1, sizeof(struct_member_t));
  member->type = type;
  member->name = name;
  return member;
}

type_t *struct_of(char *tag, struct_member_t *members) {
  type_t *type = new_type(TYPE_STRUCT);
  type->value.struct_.tag = tag;
  type->value.struct_.members = members;
  return type;
}

int type_size(type_t *type) {
  switch (type->kind) {
  case TYPE_CHAR:
    return 1;
  case TYPE_INT:
    return 4;
  case TYPE_PTR:
    return 8;
  case TYPE_ARRAY:
    return type_size(type->value.array.elm) * type->value.array.len;
  case TYPE_STRUCT: {
    int size = 0;
    struct_member_t *cur = type->value.struct_.members;
    while (cur) {
      size = align_to(size + type_size(cur->type), type_align(cur->type));
      cur = cur->next;
    }
    return size;
  }

  default:
    panic("unknown type: type=%d\n", type->kind);
  }
}

int type_align(type_t *type) {
  switch (type->kind) {
  case TYPE_CHAR:
    return 1;
  case TYPE_INT:
    return 4;
  case TYPE_PTR:
  case TYPE_ARRAY:
    return 8;
  case TYPE_STRUCT: {
    int align = 1;
    struct_member_t *cur = type->value.struct_.members;
    while (cur) {
      int cur_align = type_align(cur->type);
      if (cur_align > align) {
        align = cur_align;
      }
      cur = cur->next;
    }
    return align;
  }
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

int is_integer(type_t *type) {
  return type->kind == TYPE_CHAR || type->kind == TYPE_INT;
}

int is_ptr(type_t *type) {
  return type->kind == TYPE_PTR || type->kind == TYPE_ARRAY;
}

int align_to(int n, int align) { return (n + align - 1) & ~(align - 1); }
