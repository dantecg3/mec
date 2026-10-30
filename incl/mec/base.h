#ifndef MEC_BASE_H
#define MEC_BASE_H

// STD C99
//
#if defined(__linux__) && !defined(_DEFAULT_SOURCE)
    #define _DEFAULT_SOURCE
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

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

#if defined(__GNUC__) || defined(__clang__)
    #define $unused __attribute__((unused))
#else
    #define $unused
#endif

#define $panic(msg) do { \
        fprintf(stderr, "[%s:%i] $panic: %s\n", __FILE__, __LINE__, (msg)); \
        fflush(stderr); \
        abort(); \
    } while (0)

#ifdef MEC_RUNTIME_ASSERTS
    #define $assert(cond, msg) do { if (!(cond)) $panic("assertion failed: [" #cond "] '" msg "'"); } while (0)
#else
    #define $assert(cond, msg) ((void)0)
#endif

#define $static_assert(cond, name) typedef char static_assert_failed_##name[(cond) ? 1 : -1] $unused

#if defined(_MSC_VER)
    #define $thread __declspec(thread)
#else
    #define $thread __thread
#endif

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
$static_assert(sizeof(void*) == 8, pointer_is_64_bits);
$static_assert(sizeof(usize) == 8, usize_is_64_bits);

#endif // MEC_BASE_H
