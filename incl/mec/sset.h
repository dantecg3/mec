#ifndef MEC_SSET_H
#define MEC_SSET_H

#include "base.h"
#include "mem.h"

typedef void *SSET;

SSET  sset_new (usize stride, usize capacity, MEM_Allocator alloc);
void  sset_free (SSET *p_sset);
void  sset_insert (SSET sset, uint32 key, const void *p_val, usize val_size);
void  sset_remove (SSET sset, uint32 key);
void *sset_get (SSET sset, uint32 key, usize val_size);
void  sset_dense (SSET sset, MEM_View *out_dense, MEM_View *out_keys);

#ifdef MEC_IMPL
typedef struct
{
  uint32       *sparse;
  uint32       *keys;
  byte         *dense;
  usize         stride;
  usize         capacity;
  usize         count;
  MEM_Allocator alloc;
} SSET_Header;

static inline SSET_Header *sset_header (SSET sset)
{
  return (SSET_Header *)sset;
}

SSET sset_new (usize stride, usize capacity, MEM_Allocator alloc)
{
  uint32 *sparse = $mem_new_array (uint32, capacity, alloc);
  if (sparse == NULL)
    return NULL;

  uint32 *keys = $mem_new_array (uint32, capacity, alloc);
  if (keys == NULL)
    {
      $mem_free (&sparse, alloc);
      return NULL;
    }

  byte *dense = $mem_alloc_bytes (stride * capacity, alloc);
  if (dense == NULL)
    {
      $mem_free (&keys, alloc);
      $mem_free (&sparse, alloc);
      return NULL;
    }

  SSET_Header *header = $mem_new (SSET_Header, alloc);
  if (header == NULL)
    {
      $mem_free (&dense, alloc);
      $mem_free (&keys, alloc);
      $mem_free (&sparse, alloc);
      return NULL;
    }

  header->sparse   = sparse;
  header->keys     = keys;
  header->dense    = dense;
  header->stride   = stride;
  header->capacity = capacity;
  header->count    = 0;
  header->alloc    = alloc;

  memset (sparse, 0, capacity * sizeof (uint32));

  return (SSET)header;
}

void sset_free (SSET *p_sset)
{
  if (*p_sset == NULL)
    return;

  SSET_Header  *header = sset_header (*p_sset);
  MEM_Allocator alloc  = header->alloc;
  $mem_free (&header->sparse, alloc);
  $mem_free (&header->keys, alloc);
  $mem_free (&header->dense, alloc);
  $mem_free (p_sset, alloc);
}

void sset_insert (SSET sset, uint32 key, const void *p_val, usize val_size)
{
  if (sset == NULL || p_val == NULL || val_size == 0)
    return;

  SSET_Header *header = sset_header (sset);
  if (val_size != header->stride || key >= header->capacity)
    return;

  void *existing = sset_get (sset, key, val_size);
  if (existing != NULL)
    {
      memcpy (existing, p_val, val_size);
      return;
    }

  memcpy (&header->dense[header->count * header->stride], p_val, val_size);
  header->keys[header->count] = key;
  header->sparse[key]         = ++header->count;
}

void sset_remove (SSET sset, uint32 key)
{
  if (sset == NULL)
    return;

  SSET_Header *header = sset_header (sset);
  if (key >= header->capacity)
    return;

  uint32 slot = header->sparse[key];
  if (slot == 0)
    return;

  usize index = (slot - 1);
  usize last  = header->count - 1;

  if (index < last)
    {
      memcpy (&header->dense[index * header->stride], &header->dense[last * header->stride], header->stride);
      uint32 moved          = header->keys[last];
      header->keys[index]   = moved;
      header->sparse[moved] = slot;
    }

  header->count--;
  header->sparse[key] = 0;
}

void *sset_get (SSET sset, uint32 key, usize val_size)
{
  if (sset == NULL)
    return NULL;

  SSET_Header *header = sset_header (sset);
  if (val_size != header->stride || key >= header->capacity)
    return NULL;

  uint32 slot = header->sparse[key];
  if (slot == 0)
    return NULL;

  return (void *)(&header->dense[(slot - 1) * header->stride]);
}

void sset_dense (SSET sset, MEM_View *out_dense, MEM_View *out_keys)
{
  if (sset == NULL)
    return;

  SSET_Header *header = sset_header (sset);

  if (out_dense)
    {
      *out_dense = mem_view (header->dense, header->stride, header->count);
    }

  if (out_keys)
    {
      *out_keys = $mem_view (uint32, header->keys, header->count);
    }
}
#endif // MEC_INCL

#define $sset_new(T, capacity, alloc)  sset_new (sizeof (T), (capacity), (alloc))
#define $sset_insert(sset, key, p_val) sset_insert ((sset), (key), (const void *)(p_val), sizeof (*p_val))
#define $sset_get(T, sset, key)        (T *)sset_get ((sset), (key), sizeof (T))

#endif // MEC_SSET_H
