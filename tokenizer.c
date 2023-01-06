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
  int c = fgetc(fp);

  if (c == EOF) {
    token_t *token = calloc(1, sizeof(token_t));
    token->type = TOKEN_EOF;
    return token;
  }

  if (isdigit(c)) {
    ungetc(c, fp);
    return read_number_token(fp);
  }

  error("unexpected char");
}

token_t *tokenize(FILE *fp) {
  token_t *head = read_next_token(fp);
  token_t *cur = head;

  while (cur->type != TOKEN_EOF) {
    cur->next = (struct token_t *)read_next_token(fp);
    cur = (token_t *)cur->next;
  }

  return head;
}
