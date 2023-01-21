#include "tokenizer.h"
#include "error.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

pos_t *new_pos(int line, int column) {
  pos_t *pos = calloc(1, sizeof(pos_t));
  pos->line = line;
  pos->column = column;
  return pos;
}

pos_t *copy_pos(pos_t *pos) { return new_pos(pos->line, pos->column); }

char *pos_to_string(pos_t *pos) {
  char *buf = malloc(8);
  snprintf(buf, 8, "%d:%d", pos->line, pos->column);
  return buf;
}

tokenizer_ctx_t *new_tokenizer_ctx(FILE *fp) {
  tokenizer_ctx_t *ctx = calloc(1, sizeof(tokenizer_ctx_t));
  ctx->fp = fp;
  ctx->cur_pos = new_pos(1, 1);
  return ctx;
}

token_t *new_token(tokentype_t type, pos_t *pos) {
  token_t *token = calloc(1, sizeof(token_t));
  token->type = type;
  token->pos = pos;
  return token;
}

int read_char(tokenizer_ctx_t *ctx) {
  int c = fgetc(ctx->fp);
  if (c != '\n') {
    ctx->cur_pos->column++;
  } else {
    ctx->cur_pos->line++;
    ctx->cur_pos->column = 1;
  }
  return c;
}

void unread_char(tokenizer_ctx_t *ctx, int c) {
  ungetc(c, ctx->fp);
  ctx->cur_pos->column--; // TODO
}

void skip_whitespaces(tokenizer_ctx_t *ctx) {
  int c = read_char(ctx);
  while (isspace(c)) {
    c = read_char(ctx);
  }
  unread_char(ctx, c);
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
  } else if (!strcmp(ident, "union")) {
    token->type = TOKEN_UNION;
  } else if (!strcmp(ident, "enum")) {
    token->type = TOKEN_ENUM;
  } else if (!strcmp(ident, "void")) {
    token->type = TOKEN_VOID;
  }
}

token_t *read_ident_token(tokenizer_ctx_t *ctx) {
  pos_t *pos = copy_pos(ctx->cur_pos);
  char buf[256];
  int buf_index = 0;

  char c;
  while (1) {
    c = read_char(ctx);
    if (!is_ident_char(c)) {
      break;
    }

    buf[buf_index] = c;
    buf_index++;
  }
  unread_char(ctx, c);
  buf[buf_index] = 0;
  buf_index++;

  token_t *token = new_token(TOKEN_IDENT, pos);
  token->value.ident = calloc(1, buf_index);
  strncpy(token->value.ident, buf, buf_index);

  replace_reserved_tokens(token);

  return token;
}

token_t *read_number_token(tokenizer_ctx_t *ctx) {
  pos_t *pos = copy_pos(ctx->cur_pos);
  char buf[256];
  int buf_index = 0;

  char c;
  while (1) {
    c = read_char(ctx);
    if (!isdigit(c)) {
      break;
    }

    buf[buf_index] = c;
    buf_index++;
  }
  unread_char(ctx, c);
  buf[buf_index] = 0;

  token_t *token = new_token(TOKEN_NUMBER, pos);
  token->value.number = atoi(buf);
  return token;
}

char read_char_literal(tokenizer_ctx_t *ctx, int *is_escaped) {
  if (is_escaped) {
    *is_escaped = 0;
  }

  char c = read_char(ctx);
  if (c != '\\') {
    return c;
  }

  if (is_escaped) {
    *is_escaped = 1;
  }
  char c2 = read_char(ctx);
  switch (c2) {
  case 'n':
    return '\n';
  case '\\':
    return '\\';
  case '\'':
    return '\'';
  case '\"':
    return '\"';
  default:
    error(ctx->cur_pos, "unknown escape literal");
  }
}

token_t *read_string_token(tokenizer_ctx_t *ctx) {
  pos_t *pos = copy_pos(ctx->cur_pos);
  char buf[256];
  int buf_index = 0;

  int c, is_escaped;
  while (1) {
    c = read_char_literal(ctx, &is_escaped);
    if (c == EOF) {
      error(pos, "missing terminating '\"'\n");
    }
    if (!is_escaped && c == '"') {
      break;
    }

    buf[buf_index] = c;
    buf_index++;
  }
  buf[buf_index] = 0;
  buf_index++;

  token_t *token = new_token(TOKEN_STRING, pos);
  token->value.ident = calloc(1, buf_index);
  strncpy(token->value.string, buf, buf_index);

  return token;
}

token_t *read_char_token(tokenizer_ctx_t *ctx) {
  pos_t *pos = copy_pos(ctx->cur_pos);
  token_t *token = new_token(TOKEN_CHAR_LIT, pos);
  token->value.char_ = read_char_literal(ctx, NULL);
  if (read_char(ctx) != '\'') {
    error(pos, "missing terminating '\''\n");
  }
  return token;
}

void read_line_comment(tokenizer_ctx_t *ctx) {
  while (read_char(ctx) != '\n')
    ;
}

token_t *read_next_token(tokenizer_ctx_t *ctx) {
  skip_whitespaces(ctx);

  pos_t *pos = copy_pos(ctx->cur_pos);
  int c = read_char(ctx);

  if (c == EOF) {
    return new_token(TOKEN_EOF, pos);
  }

  if (isdigit(c)) {
    unread_char(ctx, c);
    return read_number_token(ctx);
  }

  if (is_ident_head_char(c)) {
    unread_char(ctx, c);
    return read_ident_token(ctx);
  }

  if (c == '"') {
    return read_string_token(ctx);
  }
  if (c == '\'') {
    return read_char_token(ctx);
  }

  switch (c) {
  case '(':
    return new_token(TOKEN_PAREN_OPEN, pos);
  case ')':
    return new_token(TOKEN_PAREN_CLOSE, pos);
  case ';':
    return new_token(TOKEN_SEMICOLON, pos);
  case '{':
    return new_token(TOKEN_BRACE_OPEN, pos);
  case '}':
    return new_token(TOKEN_BRACE_CLOSE, pos);
  case ',':
    return new_token(TOKEN_COMMA, pos);
  case '[':
    return new_token(TOKEN_BRACK_OPEN, pos);
  case ']':
    return new_token(TOKEN_BRACK_CLOSE, pos);
  case '~':
    return new_token(TOKEN_NOT, pos);
  case ':':
    return new_token(TOKEN_COLON, pos);
  case '.': {
    char c2 = read_char(ctx);
    if (c2 == '.') {
      char c3 = read_char(ctx);
      if (c3 == '.') {
        return new_token(TOKEN_VARARG, pos);
      }
      unread_char(ctx, c3);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_MEMBER, pos);
  }
  case '+': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_ADDEQ, pos);
    } else if (c2 == '+') {
      return new_token(TOKEN_INC, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_ADD, pos);
  }
  case '-': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_SUBEQ, pos);
    } else if (c2 == '-') {
      return new_token(TOKEN_DEC, pos);
    } else if (c2 == '>') {
      return new_token(TOKEN_ARROW, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_SUB, pos);
  }
  case '*': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_MULEQ, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_MUL, pos);
  }
  case '/': {
    char c2 = read_char(ctx);
    if (c2 == '/') {
      read_line_comment(ctx);
      return read_next_token(ctx);
    } else if (c2 == '=') {
      return new_token(TOKEN_DIVEQ, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_DIV, pos);
  }
  case '%': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_REMEQ, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_REM, pos);
  }
  case '&': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_ANDEQ, pos);
    } else if (c2 == '&') {
      return new_token(TOKEN_LOGAND, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_AND, pos);
  }
  case '|': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_OREQ, pos);
    } else if (c2 == '|') {
      return new_token(TOKEN_LOGOR, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_OR, pos);
  }
  case '^': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_XOREQ, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_XOR, pos);
  }
  case '<': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_LE, pos);
    } else if (c2 == '<') {
      char c3 = read_char(ctx);
      if (c3 == '=') {
        return new_token(TOKEN_SHLEQ, pos);
      }
      unread_char(ctx, c3);
      return new_token(TOKEN_SHL, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_LT, pos);
  }
  case '>': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_GE, pos);
    } else if (c2 == '>') {
      char c3 = read_char(ctx);
      if (c3 == '=') {
        return new_token(TOKEN_SHREQ, pos);
      }
      unread_char(ctx, c3);
      return new_token(TOKEN_SHR, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_GT, pos);
  }
  case '=': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_EQ, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_ASSIGN, pos);
  }
  case '!': {
    char c2 = read_char(ctx);
    if (c2 == '=') {
      return new_token(TOKEN_NE, pos);
    }
    unread_char(ctx, c2);
    return new_token(TOKEN_NEG, pos);
  }
  }

  error(pos, "unexpected char '%c'\n", c);
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
