#!/bin/sh
# crustpp.sh — transpile a Rust-ish .rs file to C with awk, then compile it.
# This is the "at any cost" path: unlike the C preprocessor, awk/sed can
# rewrite ARBITRARY tokens (`->`, `:` in params, `println!`), not just
# identifiers.
#
#   ./crustpp.sh examples/real_rust.rs     -> builds ./real_rust
set -eu
SRC="${1:?usage: crustpp.sh file.rs}"
OUT="${SRC%.rs}"
GEN="${OUT}.gen.c"

awk -f tools/crustpp.awk "$SRC" > "$GEN"
echo "crustpp: $SRC -> $GEN (transpiled)"
gcc -std=gnu11 -Wall -Wextra "$GEN" -o "$OUT"
echo "crustpp: $GEN -> $OUT (compiled)"
