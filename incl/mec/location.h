#ifndef MEC_LOCATION_H
#define MEC_LOCATION_H

#include "base.h"

typedef struct LOCATION LOCATION;

struct LOCATION
{
  const char *file;
  const char *func;
  int32       line;
};

#define $location() ((LOCATION){ __FILE__, __func__, __LINE__ })

#endif // MEC_LOCATION_H
