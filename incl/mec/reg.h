#ifndef MEC_REG_H
#define MEC_REG_H

#include "base.h"
#include "context.h"
#include "mem.h"
#include "bitset.h"

typedef uint32 REG_Handle;
typedef void  *REG;

#define REG_INVALID_HANDLE ((REG_Handle)0xFFFFFFFF)

static inline REG_Handle reg_handle (uint16 index, uint16 generation)
{
  return ((uint32)generation << 16) | (uint32)index;
}

static inline uint16 reg_handle_index (REG_Handle handle)
{
  return (uint16)(handle & 0xFFFF);
}

static inline uint16 reg_handle_generation (REG_Handle handle)
{
  return (uint16)(handle >> 16);
}

REG        reg_new (usize capacity, MEM_Allocator alloc);
void       reg_free (REG *p_reg);
bool       reg_is_valid_handle (REG reg, REG_Handle handle);
REG_Handle reg_get (REG reg, uint16 index);
REG_Handle reg_register (REG reg);
void       reg_unregister (REG reg, REG_Handle handle);
usize      reg_count (REG reg);

#ifdef MEC_IMPL

#define REG_MAX_CAPACITY   0xFFFF
#define REG_MAX_GENERATION 0xFFFE
#define REG_RETIRED        0xFFFF

typedef struct REG_Header
{
  uint64       *alive;
  uint16       *generations;
  uint16       *free_indices;
  usize         capacity;
  usize         free_count;
  usize         retired_count;
  usize         next_index;
  MEM_Allocator alloc;
} REG_Header;

REG reg_new (usize capacity, MEM_Allocator alloc)
{
  $assert (capacity != 0 && capacity <= REG_MAX_CAPACITY, "");

  // [HEADER][ALIVE][GENERATIONS][FREE_INDICES]
  usize alive_words         = $bitset (uint64, capacity);
  usize alive_offset        = sizeof (REG_Header);
  usize generations_offset  = alive_offset + sizeof (uint64) * alive_words;
  usize free_indices_offset = generations_offset + sizeof (uint16) * capacity;
  usize total               = free_indices_offset + sizeof (uint16) * capacity;

  byte *block = mem_alloc_bytes (total, alloc);
  if (block == NULL)
    {
      $error_stack_push (error (ERROR_OUT_OF_MEMORY, "failed to allocate reg block"));
      return NULL;
    }

  REG_Header *header    = (REG_Header *)block;
  header->alive         = (uint64 *)(block + alive_offset);
  header->generations   = (uint16 *)(block + generations_offset);
  header->free_indices  = (uint16 *)(block + free_indices_offset);
  header->capacity      = capacity;
  header->free_count    = 0;
  header->retired_count = 0;
  header->next_index    = 0;
  header->alloc         = alloc;

  memset (header->alive, 0, sizeof (uint64) * alive_words);
  memset (header->generations, 0, sizeof (uint16) * capacity);

  return (REG)header;
}

void reg_free (REG *p_reg)
{
  $assert (p_reg != NULL && *p_reg != NULL, "");

  REG_Header   *header = (REG_Header *)(*p_reg);
  MEM_Allocator alloc  = header->alloc;
  $mem_free (p_reg, alloc);
}

bool reg_is_valid_handle (REG reg, REG_Handle handle)
{
  $assert (reg != NULL, "");

  if (handle == REG_INVALID_HANDLE)
    return false;

  REG_Header *header = (REG_Header *)reg;

  uint16 index = reg_handle_index (handle);
  if ((usize)index >= header->next_index)
    return false;

  return $bitset_has (header->alive, index) && reg_handle_generation (handle) == header->generations[index];
}

REG_Handle reg_get (REG reg, uint16 index)
{
  $assert (reg != NULL, "");

  REG_Header *header = (REG_Header *)reg;
  if ((usize)index >= header->next_index || !$bitset_has (header->alive, index))
    return REG_INVALID_HANDLE;

  return reg_handle (index, header->generations[index]);
}

REG_Handle reg_register (REG reg)
{
  $assert (reg != NULL, "");

  REG_Header *header = (REG_Header *)reg;

  uint16 index;
  if (header->free_count > 0)
    {
      index = header->free_indices[--header->free_count];
    }
  else
    {
      if (header->next_index >= header->capacity)
        return REG_INVALID_HANDLE;

      index = (uint16)header->next_index++;
    }

  $bitset_set (header->alive, index);
  return reg_handle (index, header->generations[index]);
}

void reg_unregister (REG reg, REG_Handle handle)
{
  $assert (reg != NULL, "");

  if (!reg_is_valid_handle (reg, handle))
    return;

  REG_Header *header = (REG_Header *)reg;
  uint16      index  = reg_handle_index (handle);

  $bitset_clear (header->alive, index);

  if (header->generations[index] >= REG_MAX_GENERATION)
    {
      header->generations[index] = REG_RETIRED;
      header->retired_count++;
      return;
    }

  header->generations[index]++;
  header->free_indices[header->free_count++] = index;
}

usize reg_count (REG reg)
{
  $assert (reg != NULL, "");
  REG_Header *header = (REG_Header *)reg;
  return header->next_index - header->free_count - header->retired_count;
}

#endif // MEC_IMPL
#endif // MEC_REG_H
