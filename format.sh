#!/bin/sh
set -e

if [ -d "./mec" ]; then
    echo "Formatting mec..."
    find ./mec -name "*.c" -o -name "*.h" | xargs -r clang-format -i -style=file
    echo "mec formatted!"
else
    echo "Error: ./mec directory not found." >&2
    exit 1
fi
