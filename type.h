#pragma once

typedef enum {
  TYPE_CHAR,
  TYPE_INT,
  TYPE_PTR,
  TYPE_ARRAY,
} typekind_t;

typedef struct _type_t type_t;
struct _type_t {
  typekind_t kind;
  union {
    type_t *ptr;
    struct {
      type_t *elm;
      int len;
    } array;
  } value;
};

type_t *new_type(typekind_t kind);

type_t *ptr_to(type_t *base_type);

type_t *array_of(type_t *elm_type, int len);

int type_size(type_t *type);

int type_align(type_t *type);

type_t *type_deref(type_t *type);

int is_integer(type_t *type);

int is_ptr(type_t *type);

int align_to(int n, int align);
