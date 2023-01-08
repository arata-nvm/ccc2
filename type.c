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

int type_size(type_t *type) {
  switch (type->kind) {
  case TYPE_INT:
    return 4;
  case TYPE_PTR:
    return 8;
  }

  error("unknown type");
}
