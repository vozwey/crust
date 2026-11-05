#!/bin/sh
# crustc — the "Rust compiler" written in shell + the C preprocessor.
#
#   ./crustc.sh examples/hello.crust        -> builds ./hello
#
# Pipeline:  .crust --(gcc -E -P: #include + macro expansion)--> .gen.c --(gcc)--> binary
# The preprocessor is the whole compiler frontend: it reads the input file,
# inlines crust.h, and rewrites every identifier-level Rust-ism into C.
set -eu
SRC="${1:?usage: crustc.sh file.crust}"
OUT="${SRC%.crust}"
GEN="${OUT}.gen.c"

gcc -std=gnu11 -I. -E -P -x c "$SRC" -o "$GEN"
echo "crustc: $SRC -> $GEN (preprocessed)"
gcc -std=gnu11 -Wall -Wextra "$GEN" -o "$OUT"
echo "crustc: $GEN -> $OUT (compiled)"
