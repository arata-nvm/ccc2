#include "error.h"
#include "tokenizer.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void panic(char *format, ...) {
  va_list args;
  va_start(args, format);
  // vfprintf(stderr, format, args);
  vprintf(format, args);
  va_end(args);

  exit(1);
}

void error(pos_t *pos, char *format, ...) {
  if (pos != NULL) {
    // fprintf(stderr, "%s; ", pos_to_string(pos));
    printf("%s; ", pos_to_string(pos));
  }

  va_list args;
  va_start(args, format);
  // vfprintf(stderr, format, args);
  vprintf(format, args);
  va_end(args);

  exit(1);
}
