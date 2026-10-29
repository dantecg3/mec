#define MECH_IMPL
#include "mech/mem.h"
#include "mech/co.h"

#include <stdio.h>

typedef struct {
    const char *name;
    uint32 wait;
} Entity;

static void entity_proc(void *arg)
{
    Entity *e = (Entity*)arg;

    printf("%s: spawned\n", e->name);

    for (uint32 i = 0; i < e->wait; ++i) // TODO add co_sleep
    {
        printf("%s: waiting %u/%u\n", e->name, i + 1, e->wait);
        co_yield();
    }

    printf("%s: done\n", e->name);
}

int main(void)
{
    Entity entities[3] = {
        { "alice", 2 },
        { "bob",   4 },
        { "carol", 1 },
    };

    CO_Co coroutines[3];
    for (int i = 0; i < 3; ++i)
    {
        coroutines[i] = co_spawn(entity_proc, &entities[i], MEM_DEFAULT_ALLOCATOR);
        if (coroutines[i] == NULL) panic__("failed to alloc couroutines");
    }

    co_run(coroutines, 3);

    for (int i = 0; i < 3; ++i) co_free(&coroutines[i]);
    return 0;
}
