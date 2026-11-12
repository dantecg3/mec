#ifndef MEC_MEM
#define MEC_MEM

#include "base.h"
#include "range.h"
#include "sys.h"

#define MEM_KB ((usize)1 << 10)
#define MEM_MB ((usize)1 << 20)
#define MEM_GB ((usize)1 << 30)

#define MEM_DEFAULT_ALIGNMENT 16

typedef struct MEM_Slice     MEM_Slice;
typedef struct MEM_View      MEM_View;
typedef struct MEM_Allocator MEM_Allocator;
typedef void *(*MEM_Alloc_Proc) (void *context, usize size);
typedef void *(*MEM_Realloc_Proc) (void *context, void *thing, usize new_size);
typedef void (*MEM_Free_Proc) (void *context, void **p_thing);
typedef struct MEM_Arena MEM_Arena;

struct MEM_Slice
{
  const byte *data;
  usize       size;
};

static inline MEM_Slice mem_slice (const void *data, RANGE range)
{
  return (MEM_Slice){
    .data = (byte *)data + range.low,
    .size = range_length (range)
  };
}

struct MEM_View
{
  const void *data;
  usize       stride;
  usize       count;
};

static inline MEM_View mem_view (const void *data, usize stride, RANGE range)
{
  return (MEM_View){
    .data   = (byte *)data + stride * range.low,
    .stride = stride,
    .count  = range_length (range)
  };
}

static inline void *mem_view_at (MEM_View view, usize index, usize type_size)
{
  $assert (index < view.count, "");
  $assert (type_size == view.stride, "");
  return (byte *)view.data + (index * view.stride);
}

#define $mem_view(T, data, range)    mem_view ((const void *)(data), sizeof (T), (range))
#define $mem_view_at(T, view, index) ((T *)mem_view_at ((view), (index), sizeof (T)))

struct MEM_Allocator
{
  void            *context;
  MEM_Alloc_Proc   alloc_proc;
  MEM_Realloc_Proc realloc_proc;
  MEM_Free_Proc    free_proc;
};

MEM_Allocator mem_default_allocator ();

void *mem_new (usize size, MEM_Allocator alloc);
void *mem_new_array (usize stride, usize count, MEM_Allocator alloc);
byte *mem_alloc_bytes (usize count, MEM_Allocator alloc);
byte *mem_realloc_bytes (byte *thing, usize new_count, MEM_Allocator alloc);
void  mem_free (void **p_thing, MEM_Allocator alloc);

#define $mem_new(T, alloc)              ((T *)mem_new (sizeof (T), (alloc)))
#define $mem_new_array(T, count, alloc) ((T *)mem_new_array (sizeof (T), (count), (alloc)))
#define $mem_free(p_thing, alloc)       mem_free ((void **)(p_thing), (alloc))

struct MEM_Arena
{
  byte *buffer;
  usize size;
  usize alignment;
  usize offset;
};

MEM_Arena     mem_arena (MEM_Slice buffer, usize alignment);
MEM_Allocator mem_arena_allocator (MEM_Arena *arena);
void          mem_arena_clear (MEM_Arena *arena);
usize         mem_arena_mark (MEM_Arena *arena);
void          mem_arena_recover (MEM_Arena *arena, usize mark);

#define $mem_scope_init(size, p_alloc)                                                                        \
  byte      $uid (buffer__)[(size)] __attribute__ ((aligned (MEM_DEFAULT_ALIGNMENT)));                        \
  MEM_Arena $uid (arena__) = mem_arena (mem_slice ($uid (buffer__), range_to (size)), MEM_DEFAULT_ALIGNMENT); \
  *(p_alloc)               = mem_arena_allocator (&$uid (arena__));

#ifdef MEC_IMPL
static inline void *mem_default_alloc (void $unused *context, usize size)
{
  return sys_alloc (size);
}

static inline void *mem_default_realloc (void $unused *context, void *thing, usize new_size)
{
  return sys_realloc (thing, new_size);
}

static inline void mem_default_free (void $unused *context, void **p_thing)
{
  sys_free (p_thing);
}

MEM_Allocator mem_default_allocator ()
{
  return (MEM_Allocator){
    .context      = NULL,
    .alloc_proc   = mem_default_alloc,
    .realloc_proc = mem_default_realloc,
    .free_proc    = mem_default_free
  };
}

void *mem_new (usize size, MEM_Allocator alloc)
{
  $assert (alloc.alloc_proc, "");
  return alloc.alloc_proc (alloc.context, size);
}

void *mem_new_array (usize stride, usize count, MEM_Allocator alloc)
{
  $assert (alloc.alloc_proc, "");
  return alloc.alloc_proc (alloc.context, stride * count);
}

byte *mem_alloc_bytes (usize count, MEM_Allocator alloc)
{
  $assert (alloc.alloc_proc, "");
  return alloc.alloc_proc (alloc.context, sizeof (byte) * count);
}

byte *mem_realloc_bytes (byte *thing, usize new_count, MEM_Allocator alloc)
{
  $assert (alloc.realloc_proc, "");
  return alloc.realloc_proc (alloc.context, (void *)thing, sizeof (byte) * new_count);
}

void mem_free (void **p_thing, MEM_Allocator alloc)
{
  $assert (alloc.free_proc, "");
  alloc.free_proc (alloc.context, p_thing);
}

MEM_Arena mem_arena (MEM_Slice buffer, usize alignment)
{
  $assert (buffer.data != NULL && buffer.size != 0, "");
  alignment = alignment == 0 ? 1 : alignment;

  usize base_addr  = (usize)buffer.data;
  usize align_addr = align_up (base_addr, alignment);
  usize pad        = align_addr - base_addr;

  $assert (buffer.size > pad, "");

  return (MEM_Arena){
    .buffer    = (byte *)buffer.data + pad,
    .size      = buffer.size - pad,
    .alignment = alignment,
    .offset    = 0
  };
}

static inline void *mem_arena_alloc (void *context, usize size)
{
  $assert (context != NULL && size != 0, "");

  MEM_Arena *arena = (MEM_Arena *)context;

  usize aligned_size = align_up (size, arena->alignment);
  $assert (aligned_size <= arena->size - arena->offset, "");

  void *thing = arena->buffer + arena->offset;
  arena->offset += aligned_size;

  return thing;
}

static inline void *mem_arena_realloc (void $unused *context, void $unused *thing, usize $unused new_size)
{
  $panic ("invalid operation - cannot realloc in a arena");
  return NULL;
}

static inline void mem_arena_free (void $unused *context, void **p_thing)
{
  *p_thing = NULL;
}

MEM_Allocator mem_arena_allocator (MEM_Arena *arena)
{
  return (MEM_Allocator){
    .context      = (void *)arena,
    .alloc_proc   = mem_arena_alloc,
    .realloc_proc = mem_arena_realloc,
    .free_proc    = mem_arena_free
  };
}

void mem_arena_clear (MEM_Arena *arena)
{
  $assert (arena != NULL, "");
  arena->offset = 0;
}

usize mem_arena_mark (MEM_Arena *arena)
{
  $assert (arena != NULL, "");
  return arena->offset;
}

void mem_arena_recover (MEM_Arena *arena, usize mark)
{
  $assert (arena != NULL, "");
  arena->offset = mark;
}

#endif // MEC_IMPL
#endif // MEC_MEM
