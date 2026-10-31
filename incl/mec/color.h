#ifndef MEC_COLOR_H
#define MEC_COLOR_H

#include "base.h"

typedef union {
    uint8 raw[4];
    struct {
        uint8 r, g, b, a;
    };
} COLOR;

static inline COLOR color_rgba(uint8 r, uint8 g, uint8 b, uint8 a)
{
    return (COLOR){{r, g, b, a}};
}

static inline COLOR color_rgb(uint8 r, uint8 g, uint8 b)
{
    return (COLOR){{r, g, b, 255}};
}

#endif // MEC_COLOR_H
