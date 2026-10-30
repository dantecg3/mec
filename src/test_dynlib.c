#include "mec/base.h"

#ifdef WINDOWS__
    #define EXPORT __declspec(dllexport)
#elif defined(LINUX__) || defined(MACOS__)
    #define EXPORT
#endif

typedef int (*multiply_proc)(int a, int b);

EXPORT int multiply(int a, int b)
{
    return a * b;
}
