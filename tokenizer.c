#include "tokenizer.h"
#include "error.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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

int is_ident_char(char c) { return isalnum(c) || c == '_'; }

void replace_reserved_tokens(token_t *token) {
  if (token->type != TOKEN_IDENT) {
    return;
  }

  char *ident = token->value.ident;
  if (!strcmp(ident, "return")) {
    token->type = TOKEN_RETURN;
  } else if (!strcmp(ident, "if")) {
    token->type = TOKEN_IF;
  } else if (!strcmp(ident, "else")) {
    token->type = TOKEN_ELSE;
  } else if (!strcmp(ident, "while")) {
    token->type = TOKEN_WHILE;
  } else if (!strcmp(ident, "for")) {
    token->type = TOKEN_FOR;
  } else if (!strcmp(ident, "int")) {
    token->type = TOKEN_INT;
  } else if (!strcmp(ident, "sizeof")) {
    token->type = TOKEN_SIZEOF;
  } else if (!strcmp(ident, "char")) {
    token->type = TOKEN_CHAR;
  }
}

token_t *read_ident_token(FILE *fp) {
  char buf[256];
  int buf_index = 0;

  char c;
  while (1) {
    c = fgetc(fp);
    if (!is_ident_char(c)) {
      break;
    }

    buf[buf_index] = c;
    buf_index++;
  }
  ungetc(c, fp);
  buf[buf_index] = 0;
  buf_index++;

  token_t *token = new_token(TOKEN_IDENT);
  token->value.ident = calloc(1, buf_index);
  strncpy(token->value.ident, buf, buf_index);

  replace_reserved_tokens(token);

  return token;
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

  if (isalpha(c)) {
    ungetc(c, fp);
    return read_ident_token(fp);
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
  case '%':
    return new_token(TOKEN_REM);
  case '(':
    return new_token(TOKEN_PAREN_OPEN);
  case ')':
    return new_token(TOKEN_PAREN_CLOSE);
  case ';':
    return new_token(TOKEN_SEMICOLON);
  case '{':
    return new_token(TOKEN_BRACE_OPEN);
  case '}':
    return new_token(TOKEN_BRACE_CLOSE);
  case ',':
    return new_token(TOKEN_COMMA);
  case '&':
    return new_token(TOKEN_REF);
  case '[':
    return new_token(TOKEN_BRACK_OPEN);
  case ']':
    return new_token(TOKEN_BRACK_CLOSE);
  case '<': {
    char c2 = fgetc(fp);
    if (c2 == '=') {
      return new_token(TOKEN_LE);
    }
    ungetc(c2, fp);
    return new_token(TOKEN_LT);
  }
  case '>': {
    char c2 = fgetc(fp);
    if (c2 == '=') {
      return new_token(TOKEN_GE);
    }
    ungetc(c2, fp);
    return new_token(TOKEN_GT);
  }
  case '=': {
    char c2 = fgetc(fp);
    if (c2 == '=') {
      return new_token(TOKEN_EQ);
    }
    ungetc(c2, fp);
    return new_token(TOKEN_ASSIGN);
  }
  case '!': {
    char c2 = fgetc(fp);
    if (c2 == '=') {
      return new_token(TOKEN_NE);
    }
    ungetc(c2, fp);
    break;
  }
  }

  panic("unexpected char '%c'\n", c);
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
