#!/bin/bash
set -e

mode="${1:-debug}"
flags="-std=c99 -Wall -Wextra -pedantic -I./incl"

case "$mode" in
    debug)   flags="$flags -O0 -g" ;;
    release) flags="$flags -O2 -DNDEBUG" ;;
    tsan)    flags="$flags -O1 -g -fsanitize=thread" ;;
    asan)    flags="$flags -O1 -g -fsanitize=address,undefined" ;;
    *) echo "usage: $0 [debug|release|tsan|asan]"; exit 1 ;;
esac

output="bin/test_${mode}"

echo "building 'test.c' ($mode)"

mkdir -p bin
rm -f "$output"
cc $flags src/*.c -o "$output"

echo "built $output"
