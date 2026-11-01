#ifndef MEC_RANGE_H
#define MEC_RANGE_H

#include "base.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
  usize lo;
  usize hi;
} RANGE;

#define $range(lo, hi) \
  (RANGE) { (lo), (hi) }

static inline RANGE range (usize lo, usize hi)
{
  return (RANGE){ lo, (hi < lo) ? lo : hi };
}

static inline usize range_len (RANGE range)
{
  return range.hi - range.lo;
}

static inline bool range_empty (RANGE range)
{
  return range.lo >= range.hi;
}

static inline bool range_contains (RANGE range, usize i)
{
  return i >= range.lo && i < range.hi;
}

static inline RANGE range_intersect (RANGE a, RANGE b)
{
  usize lo = (a.lo > b.lo) ? a.lo : b.lo;
  usize hi = (a.hi < b.hi) ? a.hi : b.hi;
  return range (lo, hi);
}

#endif // MEC_RANGE_H
