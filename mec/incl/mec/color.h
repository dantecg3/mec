#ifndef MEC_COLOR_H
#define MEC_COLOR_H

#include "base.h"

typedef union
{
    uint8 raw[4];
    struct
    {
        uint8 r, g, b, a;
    };
} COLOR;

static inline COLOR color_rgba (uint8 r, uint8 g, uint8 b, uint8 a)
{
    return (COLOR){ { r, g, b, a } };
}

static inline COLOR color_rgb (uint8 r, uint8 g, uint8 b)
{
    return (COLOR){ { r, g, b, 255 } };
}

static inline COLOR color_hex (uint32 hex)
{
    COLOR color = {0};
    color.r     = (hex >> 24) & 0xFF;
    color.g     = (hex >> 16) & 0xFF;
    color.b     = (hex >> 8)  & 0xFF;
    color.a     = hex         & 0xFF;
    return color;
}

static inline void color_as_f32 (COLOR color, float32 out[4])
{
    out[0] = (float32)color.r / 255.0f;
    out[1] = (float32)color.g / 255.0f;
    out[2] = (float32)color.b / 255.0f;
    out[3] = (float32)color.a / 255.0f;
}

#endif // MEC_COLOR_H
