#ifndef MEC_MEM_H
#define MEC_MEM_H

#include "base.h"
#include "range.h"
#include "sys.h"

#define MEM_KB ((usize)1 << 10)
#define MEM_MB ((usize)1 << 20)
#define MEM_GB ((usize)1 << 30)
#define MEM_DEFAULT_ALIGNMENT 16

typedef struct {
    byte *data;
    usize size;
} MEM_Slice;

MEM_Slice mem_slice(void *data, usize size);
MEM_Slice mem_slice_range(MEM_Slice slice, Range range);

typedef struct {
    byte *data;
    usize count;
    usize stride;
} MEM_View;

#define $mem_view(T, data, count) (MEM_View) {(byte*)data, count, sizeof(T)}

typedef void*(*MEM_Alloc_Proc)(void *context, usize size);
typedef void*(*MEM_Realloc_Proc)(void *context, void *thing, usize new_size);
typedef void(*MEM_Free_Proc)(void *context, void **pthing);

typedef struct {
    void *context;
    MEM_Alloc_Proc alloc_proc;
    MEM_Realloc_Proc realloc_proc;
    MEM_Free_Proc free_proc;
} MEM_Allocator;

void *mem_default_alloc_proc(void *context, usize size);
void *mem_default_realloc_proc(void *context, void *thing, usize new_size);
void mem_default_free_proc(void *context, void **pthing);

#define MEM_INVALID_ALLOCATOR (MEM_Allocator) {0}
#define MEM_DEFAULT_ALLOCATOR (MEM_Allocator) { \
    .context = NULL, \
    .alloc_proc = mem_default_alloc_proc, \
    .realloc_proc = mem_default_realloc_proc, \
    .free_proc = mem_default_free_proc, \
}

typedef struct {
    MEM_Slice buffer;
    usize alignment;
    usize offset;
} MEM_Arena;

typedef usize MEM_Arena_Mark;

void mem_arena_init(MEM_Arena *arena, MEM_Slice backing_buffer, usize alignment);
void *mem_arena_alloc(MEM_Arena *arena, usize size);
MEM_Allocator mem_arena_allocator(MEM_Arena *arena);
void mem_arena_clear(MEM_Arena *arena);

MEM_Arena_Mark mem_arena_mark(MEM_Arena *arena);
void mem_arena_recover(MEM_Arena *arena, MEM_Arena_Mark mark);
usize mem_arena_used(MEM_Arena *arena);

#define MEM_ARENA_SCOPE(parena) \
    for(MEM_Arena_Mark mark__ = mem_arena_mark(parena), *once__ = &mark__;\
        once__; \
        mem_arena_recover(parena, mark__), once__ = NULL) \

#ifdef MEC_IMPL
MEM_Slice mem_slice(void *data, usize size)
{
    return (MEM_Slice) {
        .data = (byte*)data,
        .size = size
    };
}

MEM_Slice mem_slice_range(MEM_Slice slice, Range range)
{
    range = range_intersect(range, $range(0, slice.size));
    return mem_slice(slice.data + range.lo, range_len(range));
}

void *mem_default_alloc_proc(void $unused *context, usize size)
{
    return sys_alloc(size);
}

void *mem_default_realloc_proc(void $unused *context, void *thing, usize new_size)
{
    return sys_realloc(thing, new_size);
}

void mem_default_free_proc(void $unused *context, void **pthing)
{
    sys_free(pthing);
}

void mem_arena_init(MEM_Arena *arena, MEM_Slice buffer, usize alignment)
{
    if (arena == NULL) return;
    if (alignment == 0) alignment = 1;

    usize base_addr = (usize)buffer.data;
    usize align_addr = align_up(base_addr, alignment);
    usize pad = align_addr - base_addr;

    MEM_Slice aligned_buffer = {
        .data = buffer.data + pad,
        .size = (buffer.size > pad) ? buffer.size - pad : 0
    };

    *arena = (MEM_Arena) {
        .buffer = aligned_buffer,
        .alignment = alignment,
        .offset = 0
    };
}

void *mem_arena_alloc(MEM_Arena *arena, usize size)
{
    if (arena == NULL || size == 0) return NULL;

    usize align_size = align_up(size, arena->alignment);
    if (align_size > arena->buffer.size - arena->offset) return NULL; // Out_Arena_Memory

    void *thing = arena->buffer.data + arena->offset;
    arena->offset += align_size;

    return thing;
}

static inline void *mem_arena_alloc_proc(void *context, usize size)
{
    return mem_arena_alloc((MEM_Arena*)context, size);
}

static inline void *mem_arena_realloc_proc(void $unused *context, void $unused *thing, usize $unused new_size)
{
    $panic("mem_arena_realloc_proc: invalid operation");
    return NULL;
}

static inline void mem_arena_free_proc(void $unused *context, void **pthing)
{
    *pthing = NULL;
}

MEM_Allocator mem_arena_allocator(MEM_Arena *arena)
{
    if (arena == NULL) return MEM_INVALID_ALLOCATOR;
    return (MEM_Allocator) {
        .context = (void*)arena,
        .alloc_proc = mem_arena_alloc_proc,
        .realloc_proc = mem_arena_realloc_proc,
        .free_proc = mem_arena_free_proc
    };
}

void mem_arena_clear(MEM_Arena *arena)
{
    if (arena == NULL) return;
    arena->offset = 0;
}

MEM_Arena_Mark mem_arena_mark(MEM_Arena *arena)
{
    return (arena != NULL) ? arena->offset : 0;
}

void mem_arena_recover(MEM_Arena *arena, MEM_Arena_Mark mark)
{
    if (arena == NULL) return;
    arena->offset = mark;
}

usize mem_arena_used(MEM_Arena *arena)
{
    return (arena != NULL) ? arena->offset : 0;
}

static inline void *mem_alloc_checked(MEM_Allocator allocator, usize size)
{
    if (allocator.alloc_proc == NULL) return NULL;
    return allocator.alloc_proc(allocator.context, size);
}

static inline void *mem_realloc_checked(MEM_Allocator allocator, void *thing, usize new_size)
{
    if (allocator.realloc_proc == NULL) return NULL;
    return allocator.realloc_proc(allocator.context, thing, new_size);
}

#endif // MEC_IMPL

#define $mem_alloc_bytes(size, alloc) ((byte*)mem_alloc_checked((alloc), (size)))
#define $mem_realloc_bytes(thing, new_size, alloc) ((byte*)mem_realloc_checked((alloc), (thing), (new_size)))

#define $mem_new(T, alloc) ((T*)mem_alloc_checked((alloc), sizeof(T)))
#define $mem_new_array(T, capacity, alloc) ((T*)mem_alloc_checked((alloc), sizeof(T) * (capacity)))

#define $mem_free(pthing, allocator) do { \
    MEM_Allocator alloc__ = (allocator); \
    void *thing__ = (void*)(*(pthing)); \
    if (alloc__.free_proc != NULL && thing__ != NULL) \
    { \
        alloc__.free_proc(alloc__.context, (void**)(pthing)); \
    } \
} while(0)

#endif // MEC_MEM_H
