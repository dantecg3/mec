#include "mec/mec.h"
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

    MEM_Allocator alloc = s_context.allocator;

    FS_Path_Buffer exec_directory_buffer = fs_exec_directory_path(alloc);
    if (exec_directory_buffer == NULL) $fatal("failed to get exec directory");
    FS_Path exec_directory = fs_path(exec_directory_buffer);
    $trace("proc_path:'"SV_FMT"'\n", $sv_arg(exec_directory));

    FS_Path_Buffer libs_path_buffer = fs_path_append(exec_directory, "libs", alloc);
    FS_Path_Buffer dynlib_path_buffer = fs_path_append(fs_path(libs_path_buffer), "test_dynlib.so", alloc);

    DYNLIB_Handle dynlib = dynlib_load(fs_path(dynlib_path_buffer));
    if (dynlib == NULL)
    {
        const char *err = dynlib_error();
        $fatal(
            "failed to load dynlib '"SV_FMT"':\n %s",
            $sv_arg(fs_path(dynlib_path_buffer)),
            err != NULL ? err : "unknown error"
        );
    }

    typedef int (*multiply_proc)(int a, int b);
    multiply_proc mul = $dynlib_symbol(multiply_proc, dynlib, "multiply");
    if (mul == NULL) $fatal("failed to load 'multiply' symbol");

    int c = mul(2, 3);
    $info("2 * 3 = %i", c);

    dynlib_close(dynlib);

    return 0;
}
