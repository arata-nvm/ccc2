#!/bin/bash -eux

out="tmp.c"

cat > ${out} << EOF
typedef struct FILE FILE;
EOF

grep -v '^#' type.h >> ${out}
grep -v '^#' tokenizer.h >> ${out}
grep -v '^#' error.h >> ${out}
grep -v '^#' parser.h >> ${out}
grep -v '^#' codegen.h >> ${out}
