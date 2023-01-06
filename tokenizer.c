#include "tokenizer.h"
#include "error.h"
#include <ctype.h>
#include <stdlib.h>

token_t *read_number_token(FILE *fp) {
  char buf[256];
  int buf_index = 0;

  char c;
  while (1) {
    c = fgetc(fp);
    if (!isdigit(c)) {
      break;
    }

    buf[buf_index] = c;
    buf_index++;
  }
  ungetc(c, fp);
  buf[buf_index] = 0;

  token_t *token = calloc(1, sizeof(token_t));
  token->type = TOKEN_NUMBER;
  token->value.number = atoi(buf);
  return token;
}

token_t *read_next_token(FILE *fp) {
  char c = fgetc(fp);

  if (isdigit(c)) {
    ungetc(c, fp);
    return read_number_token(fp);
  }

  error("unexpected char");
}
