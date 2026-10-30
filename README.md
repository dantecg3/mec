# Modern Engine Core

Small single-header C library for engine code: memory, strings, logging, coroutines.

Targets C99 (`-std=c99`) on GCC and clang.

## Usage

Define `MEC_IMPL` in exactly one `.c` file before including the headers:

```c
#define MEC_IMPL
#include "mec/context.h"
#include "mec/mem.h"
#include "mec/co.h"
#include "mec/log.h"
...
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
cc -std=c99 -Wall -Wextra -pedantic -I./incl src/test.c -o bin/test
```
