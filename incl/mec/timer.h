#ifndef MEC_TIMER_H
#define MEC_TIMER_H

#include "base.h"
#include "sys.h"

typedef struct TIMER TIMER;

struct TIMER
{
  uint64 start; // ns
  uint64 snap;  // ns
};

static inline TIMER timer ()
{
  uint64 now = sys_time_now ();
  return (TIMER){
    .start = now,
    .snap  = now
  };
}

uint64  timer_since_start (TIMER *p_timer);
uint64  timer_elapsed (TIMER *p_timer);
uint64  timer_delta (TIMER *p_timer);
float64 timer_delta_milis (TIMER *p_timer);
float64 timer_delta_seconds (TIMER *p_timer);

#ifdef MEC_IMPL

uint64 timer_since_start (TIMER *p_timer)
{
  $assert (p_timer != NULL, "");
  return sys_time_now () - p_timer->start;
}

uint64 timer_elapsed (TIMER *p_timer)
{
  $assert (p_timer != NULL, "");
  return sys_time_now () - p_timer->snap;
}

uint64 timer_delta (TIMER *p_timer)
{
  $assert (p_timer != NULL, "");

  uint64 now    = sys_time_now ();
  uint64 delta  = now - p_timer->snap;
  p_timer->snap = now;

  return delta;
}

float64 timer_delta_milis (TIMER *p_timer)
{
  $assert (p_timer != NULL, "");
  return (float64)timer_delta (p_timer) / 1e6;
}

float64 timer_delta_seconds (TIMER *p_timer)
{
  $assert (p_timer != NULL, "");
  return (float64)timer_delta (p_timer) / 1e9;
}
#endif // MEC_IMPL
#endif // MEC_TIMER_H
