#ifndef MECH_MAP_H
#define MECH_MAP_H

#include "base.h"
#include "mem.h"

#define MAP_EMPTY 0xFFFFFFFFu

void *map_new_impl(usize stride, usize max_entries, MEM_Allocator alloc);
void map_free(void **pmap);
bool map_put_impl(void *map, uint64 hash, const void *pval, usize val_size);
void *map_get_impl(void *map, uint64 hash, usize val_size);
usize map_count(void *map);

#define map_new(T, max_entries, alloc) (T*)map_new_impl(sizeof(T), (max_entries), (alloc))
#define map_put(map, hash, pval) map_put_impl((map), (hash), (const void*)(pval), sizeof(*(pval)))
#define map_get(map, T, hash) (T*)map_get_impl((map), (hash), sizeof(T))

#ifdef MECH_IMPL
typedef struct {
    MEM_Allocator alloc;
    uint64 *hashes;
    uint32 *index;
    usize stride;
    usize index_capacity; // power of two
    usize max_entries;
    usize count;
} MAP_Header;

static MAP_Header *map_header(void *map)
{
    return (MAP_Header*)map - 1;
}

// computes smallest power of two greater or equal to both 8 and 'n'
static inline usize map_round_pow2(usize n)
{
    usize p = 8;
    while (p < n) p <<= 1;
    return p;
}

void *map_new_impl(usize stride, usize max_entries, MEM_Allocator alloc)
{
    usize index_capacity = map_round_pow2(max_entries * 2);

    // [header][values][hashes][index]
    usize values_offset = sizeof(MAP_Header);
    usize hashes_offset = align_up(values_offset + max_entries * stride, sizeof(uint64));
    usize index_offset  = hashes_offset + max_entries * sizeof(uint64);
    usize total         = index_offset + index_capacity * sizeof(uint32);

    byte *block = mem_alloc_bytes(total, alloc);
    if (block == NULL) return NULL; // Out_Of_Memory

    MAP_Header *header = (MAP_Header*)block;
    header->alloc          = alloc;
    header->hashes         = (uint64*)(block + hashes_offset);
    header->index          = (uint32*)(block + index_offset);
    header->stride         = stride;
    header->index_capacity = index_capacity;
    header->max_entries    = max_entries;
    header->count          = 0;

    memset(header->index, 0xFF, index_capacity * sizeof(uint32));

    return block + values_offset;
}

void map_free(void **pmap)
{
    if (*pmap == NULL) return;

    MAP_Header *header = map_header(*pmap);
    MEM_Allocator alloc = header->alloc;
    mem_free(&header, alloc);
    *pmap = NULL;
}

static inline uint64 map_mix(uint64 hash)
{
    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdull;
    hash ^= hash >> 33;
    return hash;
}

static bool map_find(MAP_Header *header, uint64 hash, usize *pentry, usize *pslot)
{
    usize mask = header->index_capacity - 1;
    usize i = (usize)map_mix(hash) & mask;

    while (header->index[i] != MAP_EMPTY)
    {
        usize entry = header->index[i];
        if (header->hashes[entry] == hash)
        {
            *pentry = entry;
            *pslot = i;
            return true;
        }
        i = (i + 1) & mask;
    }

    *pslot = i;
    return false;
}

bool map_put_impl(void *map, uint64 hash, const void *pval, usize val_size)
{
    if (map == NULL) return false;

    MAP_Header *header = map_header(map);

    if (header->stride != val_size) panic__("map_put: type T invalid size");

    usize entry, slot;
    if (!map_find(header, hash, &entry, &slot))
    {
        if (header->count == header->max_entries) return false; // full

        entry = header->count++;
        header->hashes[entry] = hash;
        header->index[slot] = (uint32)entry;
    }

    memcpy((byte*)map + entry * header->stride, pval, header->stride);
    return true;
}

void *map_get_impl(void *map, uint64 hash, usize val_size)
{
    if (map == NULL) return NULL;

    MAP_Header *header = map_header(map);

    if (header->stride != val_size) panic__("map_get: type T invalid size");

    usize entry, slot;
    if (!map_find(header, hash, &entry, &slot)) return NULL;

    return (byte*)map + entry * header->stride;
}

usize map_count(void *map)
{
    return map ? map_header(map)->count : 0;
}

#endif // MECH_IMPL
#endif // MECH_MAP_H
