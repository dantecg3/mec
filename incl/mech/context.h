#ifndef MECH_CONTEXT_H
#define MECH_CONTEXT_H

#include "mem.h"
#include "log.h"

typedef struct {
    MEM_Allocator allocator;
    LOG_Logger logger;
} MECH_Context;

extern $thread MECH_Context s_context;

#ifdef MECH_IMPL
$thread MECH_Context s_context = {0};

#endif // MECH_IMPL

#define $log(level, ...) log_fmt(&s_context.logger, level, __VA_ARGS__)
#define $trace(...) $log(LOG_LEVEL_TRACE, __VA_ARGS__)
#define $debug(...) $log(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define $info(...)  $log(LOG_LEVEL_INFO,  __VA_ARGS__)
#define $warn(...)  $log(LOG_LEVEL_WARN,  __VA_ARGS__)
#define $error(...) $log(LOG_LEVEL_ERROR, __VA_ARGS__)
#define $fatal(...) $log(LOG_LEVEL_FATAL, __VA_ARGS__)

#endif // MECH_CONTEXT_H
