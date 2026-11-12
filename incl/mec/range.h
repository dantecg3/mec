#ifndef MEC_RANGE_H
#define MEC_RANGE_H

#include "base.h"

typedef struct RANGE RANGE;

struct RANGE
{
  usize low;
  usize high;
};

static inline RANGE range (usize low, usize high)
{
  return (RANGE){
    .low  = low,
    .high = (high < low) ? low : high
  };
}

static inline RANGE range_to (usize high)
{
  return (RANGE){
    .low  = 0,
    .high = high
  };
}

static inline usize range_length (RANGE range)
{
  return range.high - range.low;
}

static inline bool range_contains (RANGE range, usize index)
{
  return index >= range.low && index < range.high;
}

static inline RANGE range_intersect (RANGE a, RANGE b)
{
  usize low  = (a.low > b.low) ? a.low : b.low;
  usize high = (a.high < b.high) ? a.high : b.high;
  return range (low, high);
}

#endif // MEC_RANGE_H
