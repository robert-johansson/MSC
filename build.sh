#!/usr/bin/env bash
set -euo pipefail

SRC_FILES=(src/*.c)
echo "${SRC_FILES[@]}"

CC_BIN="${CC:-gcc}"
CFLAGS="-ffunction-sections -fdata-sections -D_POSIX_C_SOURCE=199506L -pedantic -std=c99 -g3 -O3 -Wall -Wextra -Wformat-security"
LDFLAGS="-lm"
GC_FLAGS="-Wl,--gc-sections -Wl,--print-gc-sections"

if [[ "$(uname -s)" == "Darwin" ]]; then
    GC_FLAGS="-Wl,-dead_strip"
    echo "Compilation started: dead-strip unused code (macOS toolchain)."
else
    echo "Compilation started: unused code will be printed and removed."
fi

"$CC_BIN" $CFLAGS $GC_FLAGS "${SRC_FILES[@]}" $LDFLAGS -o MSC
echo "Done."
