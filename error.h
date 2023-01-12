#pragma once
#include "tokenizer.h"

void panic(char *format, ...);

void error(pos_t *pos, char *format, ...);
