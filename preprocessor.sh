#!/bin/bash -eux

out="tmp.c"

function process {
  grep -v '^#' "$1" \
  | sed 's/NULL/0/g' \
  | sed 's/ EOF/ -1/g' \
  >> ${out}
}

cat > ${out} << EOF
typedef struct FILE FILE;
typedef int va_list;
void va_start() {}
void va_end() {}
extern FILE* stdout;
EOF

process type.h
process tokenizer.h
process error.h
process parser.h
process codegen.h

process type.c
process tokenizer.c
process error.c
process parser.c
process codegen.c
process main.c
