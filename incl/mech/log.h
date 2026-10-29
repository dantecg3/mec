#ifndef MECH_LOG_H
#define MECH_LOG_H

#include "base.h"
#include "str.h"
#include <stdarg.h>

typedef enum uint8 {
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
} LOG_Level;

typedef void (*LOG_Sink_Proc)(void *context, LOG_Level level, STR_View msg);

#define LOG_LOGGER_MAX_SINKS 6
#define LOG_LOGGER_BUFFER_CAPACITY 256

typedef struct {
    LOG_Sink_Proc proc;
    void *context;
} LOG_Sink;

typedef struct {
    LOG_Level level;
    LOG_Sink sinks[LOG_LOGGER_MAX_SINKS];
    usize sink_count;
} LOG_Logger;

bool log_add_sink(LOG_Logger *logger, LOG_Sink_Proc proc, void *context);
void log_fmt(LOG_Logger *logger, LOG_Level level, const char *fmt, ...);

#ifdef MECH_IMPL
static const char *s_log_level_names[] = {
    "trace", "debug", "info",
    "warn", "error", "fatal"
};

bool log_add_sink(LOG_Logger *logger, LOG_Sink_Proc proc, void *context)
{
    if (logger->sink_count >= LOG_LOGGER_MAX_SINKS) return false;
    logger->sinks[logger->sink_count++] = (LOG_Sink){ proc, context };
    return true;
}

void log_fmt(LOG_Logger *logger, LOG_Level level, const char *fmt, ...)
{
    if (level >= logger->level)
    {
        char buffer[LOG_LOGGER_BUFFER_CAPACITY];
        int length = snprintf(buffer, sizeof buffer, "[%s]: ", s_log_level_names[level]);

        va_list args;
        va_start(args, fmt);
        length += vsnprintf(buffer + length, sizeof buffer - length, fmt, args);
        va_end(args);

        if (length > 0)
        {
            usize message_length = ((usize)length < sizeof buffer) ? (usize)length : sizeof buffer - 1;
            STR_View message = str_view_raw(buffer, message_length);
            for (usize sink_index = 0; sink_index < logger->sink_count; ++sink_index)
            {
                LOG_Sink *sink = &logger->sinks[sink_index];
                sink->proc(sink->context, level, message);
            }
        }
    }

    if (level == LOG_LEVEL_FATAL) $panic("log_fmt: fatal log has been called");
}
#endif // MECH_IMPL
#endif // MECH_LOG_H
