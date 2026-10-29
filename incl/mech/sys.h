#ifndef MECH_SYS_H
#define MECH_SYS_H

#include "base.h"

void *sys_alloc(usize size);
void *sys_realloc(void *thing, usize new_size);
void sys_free(void **pthing);

#ifdef MECH_IMPL
void *sys_alloc(usize size)
{
    void *thing = malloc(size);
    if (thing == NULL) $panic("sys_alloc -> Out_Of_Memory");
    return thing;
}

void *sys_realloc(void *thing, usize new_size)
{
    void *new_thing = realloc(thing, new_size);
    if (thing == NULL) $panic("sys_realloc -> Out_Of_Memory");
    return new_thing;
}

void sys_free(void **pthing)
{
    if (pthing == NULL || *pthing == NULL) return;
    free(*pthing);
    *pthing = NULL;
}
#endif // MECH_IMPL
#endif // MECH_SYS_H
