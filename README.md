# Modern Engine Core Headers

Small single-header C modules for engine code. 64-bit only, no dependencies beyond the C standard library.

Targets GNU C99 (`-std=gnu99`) on GCC and clang. The headers use `__thread`, `__attribute__` and inline asm for coroutines, so strict `-std=c99 -pedantic` will warn.

## Usage

Define `MECH_IMPL` in exactly one `.c` file before including the headers:

```c
#define MECH_IMPL
#include "mech/context.h"
#include "mech/mem.h"
#include "mech/co.h"
#include "mech/log.h"
```

Every other file includes the headers without the define.

## Logging

```c
static void stdout_sink(void *context $unused, LOG_Level level $unused, STR_View msg)
{
    printf(SV_FMT "\n", $sv_arg(msg));
}

log_add_sink(&s_context.logger, stdout_sink, NULL);
$info("hello %i", 5);   // prints "[info]: hello 5"
```

Levels: `$trace`, `$debug`, `$info`, `$warn`, `$error`, `$fatal`. `$fatal` always panics after the sinks run.

## Build

```sh
cc -std=gnu99 -Wall -Wextra -I./incl src/test.c -o bin/test
```
