#include "tokenizer.h"
#include "error.h"
#include <ctype.h>
#include <stdlib.h>

token_t *new_token(tokentype_t type) {
  token_t *token = calloc(1, sizeof(token_t));
  token->type = type;
  return token;
}

int read_char(FILE *fp) {
  int c = fgetc(fp);
  while (isspace(c)) {
    c = fgetc(fp);
  }

  return c;
}

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

  token_t *token = new_token(TOKEN_NUMBER);
  token->value.number = atoi(buf);
  return token;
}

token_t *read_next_token(FILE *fp) {
  int c = read_char(fp);

  if (c == EOF) {
    return new_token(TOKEN_EOF);
  }

  if (isdigit(c)) {
    ungetc(c, fp);
    return read_number_token(fp);
  }

  switch (c) {
  case '+':
    return new_token(TOKEN_ADD);
  case '-':
    return new_token(TOKEN_SUB);
  case '*':
    return new_token(TOKEN_MUL);
  case '/':
    return new_token(TOKEN_DIV);
  }

  error("unexpected char");
}

token_t *tokenize(FILE *fp) {
  token_t *head = read_next_token(fp);
  token_t *cur = head;

  while (cur->type != TOKEN_EOF) {
    cur->next = read_next_token(fp);
    cur = cur->next;
  }

  return head;
}
