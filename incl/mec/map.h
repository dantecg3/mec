#ifndef MEC_MAP_H
#define MEC_MAP_H

#include "base.h"
#include "context.h"
#include "mem.h"
#include "str.h"

typedef void *MAP;
typedef uint64 MAP_Hash;

#define MAP_EMPTY 0xFFFFFFFFu

MAP_Hash map_hash (const byte *bytes, usize count);

MAP map_new (usize stride, usize capacity, MEM_Allocator alloc);
void map_free (MAP *p_map);
void map_put (MAP map, MAP_Hash hash, const void *p_val, usize val_size);
void *map_get (MAP map, MAP_Hash hash, usize val_size);
usize map_count (MAP map);

#define $map_hash_str(str_view) map_hash ((const byte *)((str_view).data), (str_view).length)
#define $map_put(map, hash, p_val) do { map_put ((map), (hash), (const void *)(p_val), sizeof (*p_val)); } while (0)
#define $map_get(T, map, hash) (T *)map_get ((map), (hash), sizeof (T))

#ifdef MEC_IMPL
typedef struct MAP_Header
{
  byte *values;
  MAP_Hash *hashes;
  uint32 *indices;
  usize stride;
  usize capacity;
  usize count;
  MEM_Allocator alloc;
} MAP_Header;

static inline usize map_indices_capacity (usize requested_capacity)
{
    requested_capacity *= 2;
    usize capacity = 8;
    
    while (capacity < requested_capacity)
        capacity <<= 1;
        
    return capacity;
}

static inline MAP_Hash map_mix (MAP_Hash hash)
{
  hash ^= hash >> 33;
  hash *= 0xff51afd7ed558ccdull;
  hash ^= hash >> 33;
  return hash;
}

static bool map_find (MAP_Header *header, MAP_Hash hash, usize *p_index, usize *p_slot)
{
  usize indices_capacity = map_indices_capacity (header->capacity);
  usize mask = indices_capacity - 1;
  usize i    = (usize)map_mix (hash) & mask;

  while (header->indices[i] != MAP_EMPTY)
    {
      usize index = header->indices[i];
      if (header->hashes[index] == hash)
        {
          *p_index = index;
          *p_slot  = i;
          return true;
        }
      i = (i + 1) & mask;
    }

  *p_slot = i;
  return false;
}

MAP_Hash map_hash (const byte *bytes, usize count)
{
  const uint64 FNV_PRIME = 0x100000001b3ull;
  const uint64 FNV_OFFSET_BASIS = 0xcbf29ce484222325ull;

  const byte *p = bytes;
  uint64 hash = FNV_OFFSET_BASIS;

  for (usize i = 0; i < count; ++i)
    {
      hash ^= (uint64)p[i];
      hash *= FNV_PRIME;
    }

  return map_mix ((MAP_Hash)hash);
}

MAP map_new (usize stride, usize capacity, MEM_Allocator alloc)
{
  usize indices_capacity = map_indices_capacity (capacity); 

  // [HEADER][VALUES][HASHES][INDICES]
  usize values_offset = sizeof (MAP_Header);
  usize hashes_offset = align_up (values_offset + stride * capacity, sizeof (uint64));
  usize indices_offset = hashes_offset + sizeof (uint64) * capacity;
  usize total = indices_offset + sizeof (uint32) * indices_capacity;

  byte *block = mem_alloc_bytes (total, alloc);
  if (block == NULL)
    {
      $error_stack_push (error (ERROR_OUT_OF_MEMORY, "failed to allocate map block"));
      return NULL;
    }

  MAP_Header *header = (MAP_Header *)block;
  header->values = block + values_offset;
  header->hashes = (MAP_Hash *)(block + hashes_offset);
  header->indices = (uint32 *)(block + indices_offset);
  header->stride = stride;
  header->capacity = capacity;
  header->count = 0;
  header->alloc = alloc;

  memset (header->indices, 0xFF, sizeof (uint32) * indices_capacity);

  return (MAP)header;
}

void map_free (MAP *p_map)
{
  $assert (p_map != NULL && *p_map != NULL, "");
  MAP_Header *header = (MAP_Header *)*p_map;
  MEM_Allocator alloc = header->alloc;
  $mem_free (p_map, alloc);
}

void map_put (MAP map, MAP_Hash hash, const void *p_val, usize val_size)
{
  $assert (map != NULL, "");
  $assert (p_val != NULL && val_size != 0, "");

  MAP_Header *header = (MAP_Header *)map;
  $assert (val_size == header->stride, "");

  usize index, slot;
  if (!map_find (header, hash, &index, &slot))
    {
      if (header->count >= header->capacity)
        {
          $error_stack_push (error (ERROR_COLLECTION_FULL, ""));
          return;
        }

      index = header->count++;
      header->hashes[index] = hash;
      header->indices[slot] = (uint32)index;
    }

  memcpy (header->values + header->stride * index, p_val, header->stride);
}

void *map_get (MAP map, MAP_Hash hash, usize val_size)
{
  $assert (map != NULL, "");
  $assert (val_size != 0, "");

  MAP_Header *header = (MAP_Header *)map;
  $assert (val_size == header->stride, "");

  usize index, slot;
  if (!map_find (header, hash, &index, &slot))
    return NULL;

  return header->values + header->stride * index;
}

usize map_count (MAP map)
{
  $assert (map != NULL, "");
  return ((MAP_Header *)map)->count;
}
#endif // MEC_IMPL
#endif // MEC_MAP_H
