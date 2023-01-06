#include "tokenizer.h"
#include "error.h"
#include <ctype.h>
#include <stdlib.h>

token_t *read_next_token(FILE *fp) {
  char buf[256];
  int buf_index = 0;

  for (;;) {
    char c = fgetc(fp);

    if (isdigit(c)) {
      buf[buf_index] = c;
      buf_index += 1;
      continue;
    }

    if (isspace(c) && buf_index != 0) {
      buf[buf_index] = '\0';

      token_t *token = calloc(1, sizeof(token_t));
      token->type = TOKEN_NUMBER;
      token->value.number = atoi(buf);
      return token;
    }

    error("unexpected char");
  }
}
