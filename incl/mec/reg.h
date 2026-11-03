#ifndef MEC_REG_H
#define MEC_REG_H

#include "base.h"
#include "mem.h"

typedef uint32 REG_Handle;
#define REG_INVALID_HANDLE ((REG_Handle)0xFFFFFFFF)

static inline uint16 handle_index (REG_Handle handle)
{
  return (uint16)(handle & 0xFFFF);
}

static inline uint16 handle_generation (REG_Handle handle)
{
  return (uint16)(handle >> 16);
}

typedef void *REG;

REG reg_new (usize capacity, MEM_Allocator alloc);
void reg_free (REG *p_reg);
bool reg_is_valid_handle (REG reg, REG_Handle handle);
REG_Handle reg_get_handle (REG reg, uint16 index);
REG_Handle reg_register (REG reg);
void reg_unregister (REG reg, REG_Handle handle);
usize reg_registered (REG reg);

#ifdef MEC_IMPL
#define REG_MAX_CAPACITY   0xFFFF
#define REG_MAX_GENERATION 0xFFFE
#define REG_RETIRED        0xFFFF

typedef struct
{
  uint16 *generations;
  uint16 *free_indices;
  usize capacity;
  usize free_count;
  usize next_index;
  usize registered;
  MEM_Allocator alloc;
} REG_Header;

static inline REG_Header *reg_header (REG reg)
{
  return (REG_Header *)reg;
}

static inline REG_Handle make_handle (uint16 index, uint16 generation)
{
  return ((uint32)generation << 16) | (uint32)index;
}

REG reg_new (usize capacity, MEM_Allocator alloc)
{
  if (capacity == 0 || capacity > REG_MAX_CAPACITY)
    return NULL;

  uint16 *generations = $mem_new_array (uint16, capacity, alloc);
  if (generations == NULL)
    return NULL;

  uint16 *free_indices = $mem_new_array (uint16, capacity, alloc);
  if (free_indices == NULL)
    {
      $mem_free (&generations, alloc);
      return NULL;
    }

  REG_Header *header = $mem_new (REG_Header, alloc);
  if (header == NULL)
    {
      $mem_free (&generations, alloc);
      $mem_free (&free_indices, alloc);
      return NULL;
    }

  memset (generations, 0, capacity * sizeof (uint16));

  header->generations = generations;
  header->free_indices = free_indices;
  header->capacity = capacity;
  header->free_count = 0;
  header->next_index = 0;
  header->registered = 0;
  header->alloc = alloc;

  return (REG)header;
}

void reg_free (REG *p_reg)
{
  if (p_reg == NULL || *p_reg == NULL)
    return;

  REG_Header *header = reg_header (*p_reg);
  MEM_Allocator alloc = header->alloc;
  $mem_free (&header->generations, alloc);
  $mem_free (&header->free_indices, alloc);
  $mem_free (p_reg, alloc);
}

bool reg_is_valid_handle (REG reg, REG_Handle handle)
{
  if (reg == NULL || handle == REG_INVALID_HANDLE)
    return false;

  REG_Header *header = reg_header (reg);

  uint16 index = handle_index (handle);
  if ((usize)index >= header->next_index)
    return false;

  return header->generations[index] == handle_generation (handle);
}

REG_Handle reg_get_handle (REG reg, uint16 index)
{
  if (reg == NULL)
    return REG_INVALID_HANDLE;

  REG_Header *header = reg_header (reg);
  if ((usize)index >= header->next_index || header->generations[index] == REG_RETIRED)
    return REG_INVALID_HANDLE;

  return make_handle (index, header->generations[index]);
}

REG_Handle reg_register (REG reg)
{
  if (reg == NULL)
    return REG_INVALID_HANDLE;

  REG_Header *header = reg_header (reg);

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

  header->registered++;
  return make_handle (index, header->generations[index]);
}

void reg_unregister (REG reg, REG_Handle handle)
{
  if (reg == NULL || !reg_is_valid_handle (reg, handle))
    return;

  REG_Header *header = reg_header (reg);
  uint16 index = handle_index (handle);

  header->registered--;

  if (header->generations[index] >= REG_MAX_GENERATION)
    {
      header->generations[index] = REG_RETIRED;
      return;
    }

  header->generations[index]++;
  header->free_indices[header->free_count++] = index;
}

usize reg_registered (REG reg)
{
  return reg != NULL ? reg_header (reg)->registered : 0;
}
#endif // MEC_IMPL
#endif // MEC_REG_H
