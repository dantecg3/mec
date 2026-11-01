#!/bin/sh
set -e

if [ -d "./" ]; then
    echo "Formatting mec..."
    find ./incl ./src -name "*.c" -o -name "*.h" | xargs -r clang-format -i -style=file
    echo "mec formatted!"
else
    echo "Error: directory not found." >&2
    exit 1
fi
