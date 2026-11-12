#ifndef MEC_COLOR_H
#define MEC_COLOR_H

#include "base.h"

typedef union COLOR   COLOR;
typedef struct COLOR8 COLOR8;
typedef struct COLORF COLORF;

struct COLOR8
{
  uint8 r, g, b, a;
};

struct COLORF
{
  float32 r, g, b, a;
};

union COLOR
{
  COLOR8 color8;
  uint32 hex;
  uint8  raw[4];
};

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
  COLOR color    = { 0 };
  color.color8.r = (hex >> 24) & 0xFF;
  color.color8.g = (hex >> 16) & 0xFF;
  color.color8.b = (hex >> 8) & 0xFF;
  color.color8.a = hex & 0xFF;
  return color;
}

static inline COLORF color_to_colorf (COLOR color)
{
  return (COLORF){
    .r = (float32)color.color8.r / 255.0f,
    .g = (float32)color.color8.g / 255.0f,
    .b = (float32)color.color8.b / 255.0f,
    .a = (float32)color.color8.a / 255.0f
  };
}

#endif // MEC_COLOR_H
