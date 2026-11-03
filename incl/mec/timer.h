#ifndef MEC_TIMER_H
#define MEC_TIMER_H

#include "base.h"

typedef struct
{
  uint64 start;
} TIMER;

void    timer_start (TIMER *ptimer);
uint64  timer_now (void);
uint64  timer_elapsed (TIMER *ptimer);
uint64  timer_delta (TIMER *ptimer);
float64 timer_delta_milis (TIMER *ptimer);
float64 timer_delta_seconds (TIMER *ptimer);

#ifdef MEC_IMPL
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <time.h>

void timer_start (TIMER *ptimer)
{
  if (ptimer == NULL)
    return;
  ptimer->start = timer_now ();
}

uint64 timer_now (void)
{
  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
  return (uint64)ts.tv_sec * 1000000000ull + (uint64)ts.tv_nsec;
}

uint64 timer_elapsed (TIMER *ptimer)
{
  return ptimer != NULL ? timer_now () - ptimer->start : 0;
}

uint64 timer_delta (TIMER *ptimer)
{
  if (ptimer == NULL)
    return 0;
  uint64 now    = timer_now ();
  uint64 delta  = now - ptimer->start;
  ptimer->start = now;
  return delta;
}

float64 timer_delta_milis (TIMER *ptimer)
{
  return ptimer != NULL ? (float64)timer_delta (ptimer) / 1e6 : 0.0;
}

float64 timer_delta_seconds (TIMER *ptimer)
{
  return ptimer != NULL ? (float64)timer_delta (ptimer) / 1e9 : 0.0;
}

#endif // MEC_IMPL
#endif // MEC_TIMER_H
