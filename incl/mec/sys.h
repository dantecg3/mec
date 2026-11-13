#ifndef MEC_SYS_H
#define MEC_SYS_H

#include "base.h"

#ifdef WINDOWS__
#include <windows.h>
#elif defined(LINUX__) || defined(MACOS__)
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <time.h>
#include <unistd.h>
#ifdef MACOS__
#include <mach-o/dyld.h>
#endif
#else
#error "unsupported platform"
#endif

#define SYS_MAX_PATH_SIZE 4096

void  *sys_alloc (usize size);
void  *sys_realloc (void *thing, usize new_size);
void   sys_free (void **p_thing);
uint64 sys_time_now (void);
void   sys_proc_path (char buffer[SYS_MAX_PATH_SIZE]);

#ifdef MEC_IMPL
void *sys_alloc (usize size)
{
  $assert (size != 0, "");

  void *thing = malloc (size);
  if (thing == NULL)
    $panic ("sys_alloc: failed to malloc");

  return thing;
}

void *sys_realloc (void *thing, usize new_size)
{
  $assert (thing != NULL, "");
  $assert (new_size != 0, "");

  void *new_thing = realloc (thing, new_size);
  if (new_thing == NULL && new_size > 0)
    $panic ("sys_realloc: failed to realloc");

  return new_thing;
}

void sys_free (void **p_thing)
{
  $assert (p_thing != NULL, "");

  if (*p_thing == NULL)
    return;

  free (*p_thing);
  *p_thing = NULL;
}

uint64 sys_time_now (void)
{
#ifdef WINDOWS__
  LARGE_INTEGER frequency;
  LARGE_INTEGER counter;
  QueryPerformanceFrequency (&frequency);
  QueryPerformanceCounter (&counter);
  return (uint64)(counter.QuadPart * 1000000000ull / frequency.QuadPart);
#else
  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
  return (uint64)ts.tv_sec * 1000000000ull + (uint64)ts.tv_nsec;
#endif
}

void sys_proc_path (char buffer[SYS_MAX_PATH_SIZE])
{
#ifdef WINDOWS__
  DWORD length = GetModuleFileNameA (NULL, buffer, SYS_MAX_PATH_SIZE);
  if (length == 0 || length >= SYS_MAX_PATH_SIZE)
    buffer[0] = '\0';
#elif defined(LINUX__)
  ssize_t length = readlink ("/proc/self/exe", buffer, 4096 - 1);
  if (length == -1)
    {
      buffer[0] = '\0';
      return;
    }
  buffer[length] = '\0';
#elif defined(MACOS__)
  uint32 size = SYS_MAX_PATH_SIZE;
  if (_NSGetExecutablePath (buffer, &size) != 0)
    buffer[0] = '\0';
#else
  buffer[0] = '\0';
#endif
}
#endif // MEC_IMPL
#endif // MEC_SYS_H
