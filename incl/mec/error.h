#ifndef MEC_ERROR_H
#define MEC_ERROR_H

#include "base.h"

typedef struct ERROR ERROR;
typedef uint8        ERROR_Code;

#define ERR_FMT "[0x%02X]"

enum
{
  ERROR_NONE                = 0,
  ERROR_UNKNOWN             = 1,
  ERROR_INVALID_ARGUMENT    = 2,
  ERROR_OUT_OF_BOUNDS       = 3,
  ERROR_NOT_IMPLEMENTED     = 4,
  ERROR_TIMEOUT             = 5,
  ERROR_INVALID_STATE       = 6,
  ERROR_NOT_FOUND           = 7,
  ERROR_ALREADY_EXISTS      = 8,
  ERROR_ACCESS_DENIED       = 9,
  ERROR_OPERATION_ABORTED   = 10,
  ERROR_OUT_OF_MEMORY       = 30,
  ERROR_ARENA_EXHAUSTED     = 31,
  ERROR_INVALID_ALIGNMENT   = 32,
  ERROR_CORRUPTED_BLOCK     = 33,
  ERROR_DOUBLE_FREE         = 34,
  ERROR_NULL_POINTER        = 35,
  ERROR_STACK_OVERFLOW      = 60,
  ERROR_STACK_UNDERFLOW     = 61,
  ERROR_COLLECTION_FULL     = 62,
  ERROR_HASH_COLLISION      = 63,
  ERROR_CONTEXT_FAULT       = 64,
  ERROR_THREAD_FAULT        = 65,
  ERROR_IO_DEVICE           = 90,
  ERROR_PATH_TOO_LONG       = 91,
  ERROR_FILE_CORRUPTED      = 92,
  ERROR_END_OF_FILE         = 93,
  ERROR_BUFFER_OVERFLOW     = 94,
  ERROR_INVALID_ENCODING    = 95,
  ERROR_PARSE_SYNTAX        = 96,
  ERROR_PARSE_TYPE_MISMATCH = 97,
  ERROR_MUTEX_LOCK_FAILED   = 130,
  ERROR_DEADLOCK_DETECTED   = 131,
  ERROR_THREAD_CREATION     = 132,
  ERROR_CHANNEL_CLOSED      = 133,
  ERROR_SOCKET_CREATION     = 160,
  ERROR_CONNECTION_REFUSED  = 161,
  ERROR_CONNECTION_RESET    = 162,
  ERROR_INVALID_IP          = 163,
  ERROR_PACKET_DROPPED      = 164,
};

struct ERROR
{
  ERROR_Code  code;
  const char *msg;
};

static inline ERROR error (ERROR_Code code, const char *msg)
{
  return (ERROR){
    .code = code,
    .msg  = msg
  };
}

static inline ERROR error_null ()
{
  return (ERROR){
    .code = ERROR_NONE,
    .msg  = NULL
  };
}

static inline bool error_is_null (ERROR error)
{
  return error.code == ERROR_NONE;
}

#endif // MEC_ERROR_H
