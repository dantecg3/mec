#ifndef MEC_SSET_H
#define MEC_SSET_H

#include "base.h"
#include "context.h"
#include "mem.h"

typedef void  *SSET;
typedef uint32 SSET_Key;

SSET  sset_new (usize stride, usize capacity, MEM_Allocator alloc);
void  sset_free (SSET *p_sset);
void  sset_put (SSET sset, SSET_Key key, const void *p_val, usize val_size);
void  sset_remove (SSET sset, SSET_Key key);
void *sset_get (SSET sset, SSET_Key key, usize val_size);
void  sset_dense (SSET sset, MEM_View *out_dense, MEM_View *out_keys);

#define $sset_new(T, capacity, alloc) sset_new (sizeof (T), (capacity), (alloc))
#define $sset_put(sset, key, p_val)                                     \
  do                                                                    \
    {                                                                   \
      sset_put ((sset), (key), (const void *)(p_val), sizeof (*p_val)); \
    }                                                                   \
  while (0)
#define $sset_get(T, sset, key) (T *)sset_get ((sset), (key), sizeof (T))

#ifdef MEC_IMPL
typedef struct SSET_Header
{
  uint32       *sparse;
  SSET_Key     *keys;
  byte         *dense;
  usize         stride;
  usize         capacity;
  usize         count;
  MEM_Allocator alloc;
} SSET_Header;

SSET sset_new (usize stride, usize capacity, MEM_Allocator alloc)
{
  $assert (stride != 0 && capacity != 0, "");

  // [HEADER][SPARSE][KEYS][DENSE]
  usize sparse_offset = sizeof (SSET_Header);
  usize keys_offset   = sparse_offset + sizeof (uint32) * capacity;
  usize dense_offset  = keys_offset + sizeof (SSET_Key) * capacity;
  usize total         = dense_offset + stride * capacity;

  byte *block = mem_alloc_bytes (total, alloc);
  if (block == NULL)
    {
      $error_stack_push (error (ERROR_OUT_OF_MEMORY, "failed to allocate sset block"));
      return NULL;
    }

  SSET_Header *header = (SSET_Header *)block;
  header->sparse      = (uint32 *)(block + sparse_offset);
  header->keys        = (SSET_Key *)(block + keys_offset);
  header->dense       = (byte *)(block + dense_offset);
  header->stride      = stride;
  header->capacity    = capacity;
  header->count       = 0;
  header->alloc       = alloc;

  memset (header->sparse, 0, sizeof (uint32) * capacity);

  return (SSET)header;
}

void sset_free (SSET *p_sset)
{
  $assert (p_sset != NULL && *p_sset != NULL, "");
  SSET_Header  *header = (SSET_Header *)(*p_sset);
  MEM_Allocator alloc  = header->alloc;
  $mem_free (p_sset, alloc);
}

void sset_put (SSET sset, SSET_Key key, const void *p_val, usize val_size)
{
  $assert (sset != NULL, "");
  $assert (p_val != NULL && val_size != 0, "");

  SSET_Header *header = (SSET_Header *)sset;
  $assert (val_size == header->stride, "");
  $assert (key < header->capacity, "");

  void *existing = sset_get (sset, key, val_size);
  if (existing != NULL)
    {
      memcpy (existing, p_val, val_size);
      return;
    }

  memcpy (&header->dense[header->stride * header->count], p_val, val_size);
  header->keys[header->count] = key;
  header->sparse[key]         = ++header->count;
}

void sset_remove (SSET sset, SSET_Key key)
{
  $assert (sset != NULL, "");

  SSET_Header *header = (SSET_Header *)sset;
  $assert (key < header->capacity, "");

  uint32 slot = header->sparse[key];
  if (slot == 0)
    return;

  usize index = slot - 1;
  usize last  = header->count - 1;

  if (index < last)
    {
      memcpy (
          &header->dense[index * header->stride],
          &header->dense[last * header->stride],
          header->stride
      );

      uint32 moved          = header->keys[last];
      header->keys[index]   = moved;
      header->sparse[moved] = slot;
    }

  header->count--;
  header->sparse[key] = 0;
}

void *sset_get (SSET sset, SSET_Key key, usize val_size)
{
  $assert (sset != NULL, "");

  SSET_Header *header = (SSET_Header *)sset;
  $assert (val_size == header->stride, "");
  $assert (key < header->capacity, "");

  uint32 slot = header->sparse[key];
  if (slot == 0)
    return NULL;

  return (void *)(&header->dense[(slot - 1) * header->stride]);
}

void sset_dense (SSET sset, MEM_View *out_dense, MEM_View *out_keys)
{
  $assert (sset != NULL, "");

  SSET_Header *header = (SSET_Header *)sset;

  if (out_dense)
    *out_dense = mem_view (header->dense, header->stride, range_to (header->count));

  if (out_keys)
    *out_keys = mem_view (header->keys, sizeof (uint32), range_to (header->count));
}

#endif // MEC_IMPL
#endif // MEC_SSET_H
