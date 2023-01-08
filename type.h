#pragma once

typedef enum {
  TYPE_INT,
  TYPE_PTR,
} typekind_t;

typedef struct _type_t type_t;
struct _type_t {
  typekind_t kind;
  union {
    type_t *ptr;
  } value;
};

type_t *new_type(typekind_t kind);

type_t *ptr_to(type_t *base_type);

int type_size(type_t *type);
