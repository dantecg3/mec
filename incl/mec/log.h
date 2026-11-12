#ifndef MEC_LOG_H
#define MEC_LOG_H

#include "base.h"
#include "bitset.h"
#include "location.h"
#include <stdarg.h>

#define LOG_MAX_SINKS           6
#define LOG_MSG_BUFFER_CAPACITY 128

typedef uint8 LOG_Level;
typedef void (*LOG_Sink_Proc) (void *context, LOG_Level level, const char *msg);
typedef struct LOG_Sink   LOG_Sink;
typedef struct LOG_Logger LOG_Logger;

enum
{
  LOG_TRACE = 0,
  LOG_DEBUG = 1,
  LOG_INFO  = 2,
  LOG_WARN  = 3,
  LOG_ERROR = 4,
  LOG_FATAL = 5
};

enum
{
  LOG_FLAG_LEVEL    = 0,
  LOG_FLAG_LOCATION = 1,
  LOG_FLAGS
};

struct LOG_Sink
{
  void         *context;
  LOG_Sink_Proc proc;
};

struct LOG_Logger
{
  LOG_Level level;
  uint8     flags[$bitset (uint8, LOG_FLAGS)];
  LOG_Sink  sinks[LOG_MAX_SINKS];
  uint8     sink_used[$bitset (uint8, LOG_MAX_SINKS)];
};

LOG_Logger log_logger (LOG_Level level, uint8 flags[$bitset (uint8, LOG_FLAGS)]);
void       log_add_sink (LOG_Logger *logger, LOG_Sink_Proc sink_proc, void *sink_context);
void       log_fmt (LOG_Logger *logger, LOG_Level level, LOCATION location, const char *fmt, ...);

#ifdef MEC_IMPL

LOG_Logger log_logger (LOG_Level level, uint8 flags[$bitset (uint8, LOG_FLAGS)])
{
  $assert (flags != NULL, "");

  LOG_Logger logger = (LOG_Logger){
    .level     = level,
    .flags     = { 0 },
    .sinks     = { { 0 } },
    .sink_used = { 0 }
  };

  memcpy (logger.flags, flags, LOG_FLAGS * sizeof (uint8));

  return logger;
}

void log_add_sink (LOG_Logger *logger, LOG_Sink_Proc sink_proc, void *sink_context)
{
  $assert (logger != NULL, "");
  $assert (sink_proc != NULL, "");

  LOG_Sink *sink = NULL;
  for (usize i = 0; i < LOG_MAX_SINKS; ++i)
    {
      if ($bitset_has (logger->sink_used, i))
        continue;

      sink = &logger->sinks[i];
      $bitset_set (logger->sink_used, i);
      break;
    }

  $assert (sink != NULL, "out of sinks slots");

  sink->context = sink_context;
  sink->proc    = sink_proc;
}

static inline const char *log_level_to_str (LOG_Level level)
{
  switch (level)
    {
    case LOG_TRACE:
      return "trace";
    case LOG_DEBUG:
      return "debug";
    case LOG_INFO:
      return "info";
    case LOG_WARN:
      return "warn";
    case LOG_ERROR:
      return "error";
    case LOG_FATAL:
      return "fatal";
    }
  return "unknown";
}

void log_fmt (LOG_Logger *logger, LOG_Level level, LOCATION location, const char *fmt, ...)
{
  $assert (logger != NULL, "");

  if (level < logger->level)
    return;

  char buffer[LOG_MSG_BUFFER_CAPACITY];
  int  written  = 0;
  int  capacity = LOG_MSG_BUFFER_CAPACITY;

  if ($bitset_has (logger->flags, LOG_FLAG_LOCATION))
    {
      int n = snprintf (
          buffer + written,
          capacity > written ? capacity - written : 0,
          "(%s:%i) (%s) ",
          location.file,
          location.line,
          location.func
      );

      if (n > 0)
        written += (n < capacity - written) ? n : (capacity - written - 1);
    }

  if ($bitset_has (logger->flags, LOG_FLAG_LEVEL))
    {
      int n = snprintf (
          buffer + written,
          capacity > written ? capacity - written : 0,
          "%s: ",
          log_level_to_str (level)
      );

      if (n > 0)
        written += (n < capacity - written) ? n : (capacity - written - 1);
    }

  if (fmt != NULL && written < capacity)
    {
      va_list args;
      va_start (args, fmt);
      vsnprintf (
          buffer + written,
          capacity > written ? capacity - written : 0,
          fmt,
          args
      );
      va_end (args);
    }
  else if (written < capacity)
    {
      buffer[written] = '\0';
    }

  for (usize i = 0; i < LOG_MAX_SINKS; ++i)
    {
      if (!$bitset_has (logger->sink_used, i))
        continue;

      LOG_Sink *sink = &logger->sinks[i];
      if (sink->proc)
        sink->proc (sink->context, level, buffer);
    }

  if (level == LOG_FATAL)
    $panic ("on a fatal log");
}

#endif // MEC_IMPL
#endif // MEC_LOG_H
