#pragma once

typedef enum {
  TYPE_VOID,
  TYPE_CHAR,
  TYPE_INT,
  TYPE_PTR,
  TYPE_ARRAY,
  TYPE_STRUCT,
  TYPE_UNION,
  TYPE_ENUM,
} typekind_t;

typedef struct _type_t type_t;

typedef struct _struct_member_t struct_member_t;
struct _struct_member_t {
  type_t *type;
  char *name;
  int offset;

  struct_member_t *next;
};

typedef struct _enum_t enum_t;
struct _enum_t {
  char *name;
  int value;

  enum_t *next;
};

struct _type_t {
  typekind_t kind;
  union {
    type_t *ptr;
    struct {
      type_t *elm;
      int len;
    } array;
    struct {
      char *tag;
      struct_member_t *members;

      int size;
      int align;
    } struct_union;
    struct {
      char *tag;
      enum_t *enums;
    } enum_;
  } value;
  int is_extern;
};

type_t *new_type(typekind_t kind);

type_t *ptr_to(type_t *base_type);

type_t *array_of(type_t *elm_type, int len);

struct_member_t *new_struct_member(type_t *type, char *name);

type_t *struct_of(char *tag, struct_member_t *members);

type_t *union_of(char *tag, struct_member_t *members);

enum_t *new_enum(char *name, int value);

type_t *enum_of(char *tag, enum_t *enums);

int type_size(type_t *type);

int type_align(type_t *type);

type_t *type_deref(type_t *type);

struct_member_t *find_member(type_t *type, char *name);

int is_integer(type_t *type);

int is_ptr(type_t *type);

int is_incomlete(type_t *type);

int align_to(int n, int align);
