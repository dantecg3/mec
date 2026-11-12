#ifndef MEC_DYNARR_H
#define MEC_DYNARR_H

#include "base.h"
#include "context.h"
#include "mem.h"

#define DYNARR_GROW_MUL 2

void *dynarr_new (usize stride, usize capacity, MEM_Allocator alloc);
void  dynarr_free (void **p_arr);
void  dynarr_append (void **p_arr, const void *p_val, usize val_size);
usize dynarr_length (void *arr);
usize dynarr_capacity (void *arr);

#define $dynarr_new(T, capacity, alloc) (T *)dynarr_new (sizeof (T), (capacity), (alloc))
#define $dynarr_append(p_arr, p_val)                                            \
  do                                                                            \
    {                                                                           \
      dynarr_append ((void **)(p_arr), (const void *)(p_val), sizeof (*p_val)); \
    }                                                                           \
  while (0)

#ifdef MEC_IMPL
typedef struct
{
  usize         capacity;
  usize         stride;
  usize         length;
  MEM_Allocator alloc;
} DYNARR_Header;

static DYNARR_Header *dynarr_header (void *arr)
{
  return (DYNARR_Header *)arr - 1;
}

void *dynarr_new (usize stride, usize capacity, MEM_Allocator alloc)
{
  $assert (stride != 0 && capacity != 0, "");

  usize          size   = sizeof (DYNARR_Header) + stride * capacity;
  DYNARR_Header *header = (DYNARR_Header *)mem_alloc_bytes (size, alloc);

  if (header == NULL)
    {
      $error_stack_push (error (ERROR_OUT_OF_MEMORY, "failed to allocate a dynarr"));
      return NULL;
    }

  header->capacity = capacity;
  header->stride   = stride;
  header->length   = 0;
  header->alloc    = alloc;

  return (void *)(header + 1);
}

void dynarr_free (void **p_arr)
{
  $assert (p_arr != NULL && *p_arr != NULL, "");

  DYNARR_Header *header = dynarr_header (*p_arr);
  MEM_Allocator  alloc  = header->alloc;
  $mem_free (&header, alloc);
  *p_arr = NULL;
}

void dynarr_append (void **p_arr, const void *p_val, usize val_size)
{
  $assert (p_arr != NULL && *p_arr != NULL, "");
  $assert (p_val != NULL && val_size != 0, "");

  DYNARR_Header *header = dynarr_header (*p_arr);
  if (val_size != header->stride)
    {
      $error_stack_push (error (ERROR_INVALID_ARGUMENT, "failed to append to a dynarr"));
      return;
    }

  if (header->length >= header->capacity)
    {
      usize new_capacity = header->capacity * DYNARR_GROW_MUL;
      usize new_size     = sizeof (DYNARR_Header) + new_capacity * header->stride;

      DYNARR_Header *grown = (DYNARR_Header *)mem_realloc_bytes ((byte *)header, new_size, header->alloc);
      if (grown == NULL)
        {
          $error_stack_push (error (ERROR_OUT_OF_MEMORY, "failed to realloc growing a dynarr"));
          return;
        }

      header           = grown;
      header->capacity = new_capacity;
      *p_arr           = (void *)(header + 1);
    }

  memcpy ((byte *)*p_arr + header->length * header->stride, p_val, header->stride);
  header->length++;
}

usize dynarr_length (void *arr)
{
  $assert (arr != NULL, "");
  return dynarr_header (arr)->length;
}

usize dynarr_capacity (void *arr)
{
  $assert (arr != NULL, "");
  return dynarr_header (arr)->capacity;
}
#endif
#endif // MEC_DYNARR_H
