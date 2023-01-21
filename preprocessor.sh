#!/bin/bash -eux

out="tmp.c"

function process {
  grep -v '^#' "$1" \
  | sed 's/NULL/0/g' \
  >> ${out}
}

cat > ${out} << EOF
typedef struct FILE FILE;
EOF

process type.h
process tokenizer.h
process error.h
process parser.h
process codegen.h

process type.c
