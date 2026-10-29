# Modern Engine Core Headers

Small single-header C99 modules for engine code. 64-bit only, no dependencies beyond the C standard library.

## Usage

Define `MECH_IMPL` in exactly one `.c` file before including the headers:

```c
#define MECH_IMPL
#include "mech/mem.h"
#include "mech/co.h"
...
```

Every other file includes the headers without the define.

## Build

```sh
cc -std=c99 main.c -o main
```
