#include "tokenizer.h"
#include "error.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

tokenizer_ctx_t *new_tokenizer_ctx(FILE *fp) {
  tokenizer_ctx_t *ctx = calloc(1, sizeof(tokenizer_ctx_t));
  ctx->fp = fp;
  return ctx;
}

token_t *new_token(tokentype_t type) {
  token_t *token = calloc(1, sizeof(token_t));
  token->type = type;
  return token;
}

int next_char(tokenizer_ctx_t *ctx) { return fgetc(ctx->fp); }

void push_char(tokenizer_ctx_t *ctx, int c) { ungetc(c, ctx->fp); }

void skip_whitespaces(tokenizer_ctx_t *ctx) {
  int c = next_char(ctx);
  while (isspace(c)) {
    c = next_char(ctx);
  }
  push_char(ctx, c);
}

int is_ident_head_char(char c) { return isalpha(c) || c == '_'; }

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
  } else if (!strcmp(ident, "break")) {
    token->type = TOKEN_BREAK;
  } else if (!strcmp(ident, "continue")) {
    token->type = TOKEN_CONTINUE;
  } else if (!strcmp(ident, "switch")) {
    token->type = TOKEN_SWITCH;
  } else if (!strcmp(ident, "case")) {
    token->type = TOKEN_CASE;
  } else if (!strcmp(ident, "default")) {
    token->type = TOKEN_DEFAULT;
  } else if (!strcmp(ident, "struct")) {
    token->type = TOKEN_STRUCT;
  } else if (!strcmp(ident, "typedef")) {
    token->type = TOKEN_TYPEDEF;
  }
}

token_t *read_ident_token(tokenizer_ctx_t *ctx) {
  char buf[256];
  int buf_index = 0;

  char c;
  while (1) {
    c = next_char(ctx);
    if (!is_ident_char(c)) {
      break;
    }

    buf[buf_index] = c;
    buf_index++;
  }
  push_char(ctx, c);
  buf[buf_index] = 0;
  buf_index++;

  token_t *token = new_token(TOKEN_IDENT);
  token->value.ident = calloc(1, buf_index);
  strncpy(token->value.ident, buf, buf_index);

  replace_reserved_tokens(token);

  return token;
}

token_t *read_number_token(tokenizer_ctx_t *ctx) {
  char buf[256];
  int buf_index = 0;

  char c;
  while (1) {
    c = next_char(ctx);
    if (!isdigit(c)) {
      break;
    }

    buf[buf_index] = c;
    buf_index++;
  }
  push_char(ctx, c);
  buf[buf_index] = 0;

  token_t *token = new_token(TOKEN_NUMBER);
  token->value.number = atoi(buf);
  return token;
}

token_t *read_string_token(tokenizer_ctx_t *ctx) {
  char buf[256];
  int buf_index = 0;

  int c;
  while (1) {
    c = next_char(ctx);
    if (c == EOF) {
      panic("missing terminating '\"'\n");
    }
    if (c == '"') {
      break;
    }

    buf[buf_index] = c;
    buf_index++;
  }
  buf[buf_index] = 0;
  buf_index++;

  token_t *token = new_token(TOKEN_STRING);
  token->value.ident = calloc(1, buf_index);
  strncpy(token->value.string, buf, buf_index);

  return token;
}

void read_line_comment(tokenizer_ctx_t *ctx) {
  while (next_char(ctx) != '\n')
    ;
}

token_t *read_next_token(tokenizer_ctx_t *ctx) {
  skip_whitespaces(ctx);
  int c = next_char(ctx);

  if (c == EOF) {
    return new_token(TOKEN_EOF);
  }

  if (isdigit(c)) {
    push_char(ctx, c);
    return read_number_token(ctx);
  }

  if (is_ident_head_char(c)) {
    push_char(ctx, c);
    return read_ident_token(ctx);
  }

  if (c == '"') {
    return read_string_token(ctx);
  }

  switch (c) {
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
  case '[':
    return new_token(TOKEN_BRACK_OPEN);
  case ']':
    return new_token(TOKEN_BRACK_CLOSE);
  case '~':
    return new_token(TOKEN_NOT);
  case ':':
    return new_token(TOKEN_COLON);
  case '.':
    return new_token(TOKEN_MEMBER);
  case '+': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_ADDEQ);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_ADD);
  }
  case '-': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_SUBEQ);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_SUB);
  }
  case '*': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_MULEQ);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_MUL);
  }
  case '/': {
    char c2 = next_char(ctx);
    if (c2 == '/') {
      read_line_comment(ctx);
      return read_next_token(ctx);
    } else if (c2 == '=') {
      return new_token(TOKEN_DIVEQ);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_DIV);
  }
  case '%': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_REMEQ);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_REM);
  }
  case '&': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_ANDEQ);
    } else if (c2 == '&') {
      return new_token(TOKEN_LOGAND);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_AND);
  }
  case '|': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_OREQ);
    } else if (c2 == '|') {
      return new_token(TOKEN_LOGOR);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_OR);
  }
  case '^': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_XOREQ);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_XOR);
  }
  case '<': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_LE);
    } else if (c2 == '<') {
      char c3 = next_char(ctx);
      if (c3 == '=') {
        return new_token(TOKEN_SHLEQ);
      }
      push_char(ctx, c3);
      return new_token(TOKEN_SHL);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_LT);
  }
  case '>': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_GE);
    } else if (c2 == '>') {
      char c3 = next_char(ctx);
      if (c3 == '=') {
        return new_token(TOKEN_SHREQ);
      }
      push_char(ctx, c3);
      return new_token(TOKEN_SHR);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_GT);
  }
  case '=': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_EQ);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_ASSIGN);
  }
  case '!': {
    char c2 = next_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_NE);
    }
    push_char(ctx, c2);
    return new_token(TOKEN_NEG);
  }
  }

  panic("unexpected char '%c'\n", c);
}

token_t *tokenize(FILE *fp) {
  tokenizer_ctx_t *ctx = new_tokenizer_ctx(fp);
  token_t *head = read_next_token(ctx);
  token_t *cur = head;

  while (cur->type != TOKEN_EOF) {
    cur->next = read_next_token(ctx);
    cur = cur->next;
  }

  return head;
}
