#ifndef MEC_CONTEXT_H
#define MEC_CONTEXT_H

#include "base.h"
#include "error.h"
#include "mem.h"
#include "log.h"

typedef struct MEC_Context     MEC_Context;
typedef struct MEC_Scope       MEC_Scope;
typedef struct MEC_Error_Stack MEC_Error_Stack;

#define MEC_ERROR_STACK_CAPACITY 256
struct MEC_Error_Stack
{
  ERROR stack[MEC_ERROR_STACK_CAPACITY];
  usize length;
  usize head;
};

struct MEC_Context
{
  MEM_Allocator   default_allocator;
  MEM_Allocator   temp_allocator;
  LOG_Logger      logger;
  MEC_Error_Stack error_stack;
  uint64          thread_id;
};

extern $thread MEC_Context s_context;

static inline void mec_init_context ()
{
  s_context.default_allocator = mem_default_allocator ();
  s_context.temp_allocator    = (MEM_Allocator){ 0 };
  s_context.error_stack       = (MEC_Error_Stack){ 0 };
  s_context.thread_id         = thrd_current ();

  uint8 flags[$bitset (uint8, LOG_FLAGS)] = { 0 };
  $bitset_set (flags, LOG_FLAG_LEVEL);
  $bitset_set (flags, LOG_FLAG_LOCATION);
  s_context.logger = log_logger (LOG_TRACE, flags);
}

static inline $unused void mec_push_error (ERROR err)
{
  MEC_Error_Stack *stack = &s_context.error_stack;
  $assert (stack->head < MEC_ERROR_STACK_CAPACITY, "");

  usize tail = stack->head + stack->length;
  $assert (tail < MEC_ERROR_STACK_CAPACITY, "");

  stack->stack[tail] = err;
  stack->length++;
}

static inline $unused bool mec_pop_error (ERROR *out)
{
  MEC_Error_Stack *stack = &s_context.error_stack;

  if (stack->length <= 0)
    return false;

  $assert (stack->head < MEC_ERROR_STACK_CAPACITY, "");

  *out = stack->stack[stack->head++];
  stack->length--;

  return true;
}

#define $log(level, ...) log_fmt (&s_context.logger, level, $location (), __VA_ARGS__)
#define $trace(...)      $log (LOG_TRACE, __VA_ARGS__)
#define $debug(...)      $log (LOG_DEBUG, __VA_ARGS__)
#define $info(...)       $log (LOG_INFO, __VA_ARGS__)
#define $warn(...)       $log (LOG_WARN, __VA_ARGS__)
#define $error(...)      $log (LOG_ERROR, __VA_ARGS__)
#define $fatal(...)      $log (LOG_FATAL, __VA_ARGS__)

#define $error_stack_push(err) mec_push_error (err)
#define $error_stack_pop(out)  mec_pop_error (out)

#ifdef MEC_IMPL
$thread MEC_Context s_context = { 0 };
#endif // MEC_IMPL
#endif // MEC_CONTEXT_H
