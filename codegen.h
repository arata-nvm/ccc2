#pragma once
#include "parser.h"
#include <stdio.h>

void gen_node(node_t *node, FILE *fp);

void gen_code(node_t *node, FILE *fp);
