#ifndef MECH_CO_H
#define MECH_CO_H

// TODO(co):
// - co_sleep
// - stack overflow protection
// - other architectures

#include "base.h"
#include "mem.h"

#define CO_STACK_SIZE (16 * MEM_KB)

typedef void* CO_Co;
typedef void (*CO_Proc)(void *arg);

CO_Co co_spawn(CO_Proc proc, void *arg, MEM_Allocator alloc);
void co_free(CO_Co *pco);
void co_yield(void);
void co_run(CO_Co *cos, usize count);

// implemented by platform specific file
void co_switch(void **from_sp, void *to_sp);
void *co_init_stack(byte *stack, usize size, void (*entry)(void));

#ifdef MECH_IMPL
#if X86_64__ && (LINUX__ || MACOS__)
    #include "co_sysv_x86_64.h"
#elif AARCH64__ && (LINUX__ || MACOS__)
    #include "co_aarch64.h"
#elif X86_64__ && WINDOWS__
    #include "co_windows_x86_64.h"
#else
    #error "mech co: unsupported platform"
#endif

typedef struct {
    void *sp; // saved stack pointer while suspended
    CO_Proc proc;
    void *arg;
    byte *stack;
    bool done;
    MEM_Allocator alloc;
} CO_Header;

static inline CO_Header *co_header(CO_Co co)
{
    return (CO_Header*)co;
}

static $thread CO_Co co_current; // on going coroutine
static $thread void *co_sched_sp;

static void co_entry(void)
{
    CO_Header *header = co_header(co_current);
    header->proc(header->arg);
    header->done = true;
    co_switch(&header->sp, co_sched_sp); // never resumes
}

CO_Co co_spawn(CO_Proc proc, void *arg, MEM_Allocator alloc)
{
    if (proc == NULL) $panic("co_spawn: proc is NULL");

    CO_Header *header = $mem_new(CO_Header, alloc);
    if (header == NULL) return NULL; // Out_Of_Memory

    header->stack = $mem_alloc_bytes(CO_STACK_SIZE, alloc);
    if (header->stack == NULL)
    {
        $mem_free(&header, alloc);
        return NULL; // Out_Of_Memory
    }

    header->alloc = alloc;
    header->proc  = proc;
    header->arg   = arg;
    header->done  = false;
    header->sp    = co_init_stack(header->stack, CO_STACK_SIZE, co_entry);

    return (CO_Co)header;
}

void co_free(CO_Co *pco)
{
    if (*pco == NULL) return;

    CO_Header *header = co_header(*pco);
    if (!header->done) $panic("co_free: coroutine is still alive");

    MEM_Allocator alloc = header->alloc;
    $mem_free(&header->stack, alloc);
    $mem_free(&header, alloc);
    *pco = NULL;
}

void co_yield(void)
{
    if (co_current == NULL) $panic("co_yield: called outside a coroutine");
    co_switch(&co_header(co_current)->sp, co_sched_sp);
}

void co_run(CO_Co *cos, usize count)
{
    if (co_current != NULL) $panic("co_run: called from inside a coroutine");

    usize alive = 0;
    for (usize i = 0; i < count; ++i)
    {
        if (!co_header(cos[i])->done) alive++;
    }

    while (alive > 0)
    {
        for (usize i = 0; i < count; ++i)
        {
            CO_Header *header = co_header(cos[i]);
            if (header->done) continue;

            co_current = cos[i];
            co_switch(&co_sched_sp, header->sp);
            co_current = NULL;

            if (header->done) alive--;
        }
    }
}

#endif // MECH_IMPL
#endif // MECH_CO_H
