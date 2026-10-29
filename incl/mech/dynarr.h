#ifndef MECH_DYNARR_H
#define MECH_DYNARR_H

#include "base.h"
#include "mem.h"

#define DYNARR_DEFAULT_CAPACITY 8
#define DYNARR_DEFAULT_GROW_MUL 2

void *dynarr_new(usize stride, MEM_Allocator alloc);
void dynarr_free(void **parr);
bool dynarr_append(void **parr, const void *pval, usize val_size);
usize dynarr_count(void *arr);
usize dynarr_capacity(void *arr);

#define $dynarr_new(T, alloc) (T*)dynarr_new(sizeof(T), alloc)
#define $dynarr_free(parr) dynarr_free((void**)parr)
#define $dynarr_append(parr, pval) dynarr_append((void*)(parr), (const void*)(pval), sizeof(*(pval)))

#ifdef MECH_IMPL
typedef struct {
    usize capacity;
    usize stride;
    usize count;
    MEM_Allocator alloc;
} DYNARR_Header;

static DYNARR_Header *dynarr_header(void *arr)
{
    return (DYNARR_Header*)arr - 1;
}

void *dynarr_new(usize stride, MEM_Allocator alloc)
{
    usize size = sizeof(DYNARR_Header) + stride * DYNARR_DEFAULT_CAPACITY;

    DYNARR_Header *header = (DYNARR_Header*)$mem_alloc_bytes(size, alloc);
    if (header == NULL) return NULL; // Out_Of_Memory

    header->capacity = DYNARR_DEFAULT_CAPACITY;
    header->stride   = stride;
    header->count    = 0;
    header->alloc    = alloc;

    return header + 1;
}

void dynarr_free(void **parr)
{
    if (*parr == NULL) return;

    DYNARR_Header *header = dynarr_header(*parr);
    MEM_Allocator alloc = header->alloc;
    $mem_free(&header, alloc);
    *parr = NULL;
}

bool dynarr_append(void **parr, const void *pval, usize val_size)
{
    if (*parr == NULL) return false;

    DYNARR_Header *header = dynarr_header(*parr);

    if (header->stride != val_size) $panic("dynarr_append: type T size invalid");

    if (header->count >= header->capacity)
    {
        usize capacity = header->capacity * DYNARR_DEFAULT_GROW_MUL;
        usize size     = sizeof(DYNARR_Header) + header->stride * capacity;

        DYNARR_Header *grown = (DYNARR_Header*)$mem_realloc_bytes(header, size, header->alloc);
        if (grown == NULL) return false; // Out_Of_Memory, original array untouched

        header = grown;
        header->capacity = capacity;
        *parr = header + 1;
    }

    byte *dest = (byte*)(header + 1) + header->count * header->stride;
    memcpy(dest, pval, header->stride);
    header->count++;

    return true;
}

usize dynarr_count(void *arr)
{
    return arr ? dynarr_header(arr)->count : 0;
}

usize dynarr_capacity(void *arr)
{
    return arr ? dynarr_header(arr)->capacity : 0;
}

#endif // MECH_IMPL
#endif // MECH_DYNARR_H
