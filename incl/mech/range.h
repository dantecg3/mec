#ifndef MECH_RANGE_H
#define MECH_RANGE_H

#include "base.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    usize lo;
    usize hi;
} Range;

#define $range(lo, hi) (Range) {(lo), (hi)}

static inline Range range(usize lo, usize hi)
{
    return (Range) { lo, (hi < lo) ? lo : hi };
}

static inline usize range_len(Range range)
{
    return range.hi - range.lo;
}

static inline bool range_empty(Range range)
{
    return range.lo >= range.hi;
}

static inline bool range_contains(Range range, usize i)
{
    return i >= range.lo && i < range.hi;
}

static inline Range range_intersect(Range a, Range b)
{
    usize lo = (a.lo > b.lo) ? a.lo : b.lo;
    usize hi = (a.hi < b.hi) ? a.hi : b.hi;
    return range(lo, hi);
}

#endif // MECH_RANGE_H
