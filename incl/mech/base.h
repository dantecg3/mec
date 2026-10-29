#ifndef MECH_BASE_H
#define MECH_BASE_H

// STD C99

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t byte;

typedef size_t    usize;
typedef ptrdiff_t isize;
typedef uintptr_t uintptr;
typedef intptr_t  intptr;

typedef float  float32;
typedef double float64;

#include "range.h"

#define static_assert__(cond, name) typedef char static_assert__##name[(cond) ? 1 : -1]

#define panic__(msg) do { \
        printf("panic!: %s\n", (msg)); \
        abort(); \
     } while(0)

static inline usize align_up(usize size, usize alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

// platform
#if defined(_WIN32)
    #define WINDOWS__ 1
#elif defined(__APPLE__)
    #define MACOS__ 1
#elif defined(__linux__)
    #define LINUX__ 1
#else
    #error "unsupported platform"
#endif

// arch
#if defined(__x86_64__) || defined(_M_X64)
    #define X86_64__ 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define AARCH64__ 1
#else
    #error "unsupported architecture"
#endif

// 64-bit only
static_assert__(sizeof(void*) == 8, pointer_is_64_bits);
static_assert__(sizeof(usize) == 8, usize_is_64_bits);

#endif // MECH_BASE_H
