#define MEC_IMPL
#include "mec/context.h"
#include "mec/mem.h"
#include "mec/log.h"

#include <stdio.h>

static const char *s_level_colors[] = {
    "\x1b[90m",   // TRACE: gray
    "\x1b[36m",   // DEBUG: cyan
    "\x1b[32m",   // INFO:  green
    "\x1b[33m",   // WARN:  yellow
    "\x1b[31m",   // ERROR: red
    "\x1b[1;41m", // FATAL: bold on red background
};
#define ANSI_RESET "\x1b[0m"

static void colored_stdout_sink(void *context $unused, LOG_Level level, STR_View msg)
{
    printf("%s" SV_FMT ANSI_RESET "\n", s_level_colors[level], $sv_arg(msg));
}

static void context_init(MEC_Context *context, MEM_Allocator allocator)
{
    context->allocator = allocator;
    context->logger.level = LOG_LEVEL_TRACE;
    context->logger.sink_count = 0;
    log_add_sink(&context->logger, colored_stdout_sink, NULL);
}

int main(void)
{
    context_init(&s_context, MEM_DEFAULT_ALLOCATOR);
    $info("Hello, World!");
    return 0;
}
