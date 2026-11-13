# Modern Engine Core

Small single-header C library for game engine code: memory, strings, files, data structures, logging.

Targets C99 (`-std=c99`) on GCC and clang.

## Usage

Define `MEC_IMPL` in exactly one `.c` file before including the headers:

```c
#define MEC_IMPL
#include "mec/mec.h"
```

Every other file includes the headers without the define.

## Build

```sh
./build.sh
```
