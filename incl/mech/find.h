#ifndef MECH_FIND_H
#define MECH_FIND_H

#include "base.h"
#include "mem.h"

typedef int (*FIND_Compare_Proc)(const void *target, const void *item);

void *find_binary_search(MEM_View view, const void *target, FIND_Compare_Proc compare_proc);

#ifdef MECH_IMPL
void *find_binary_search(MEM_View view, const void *target, FIND_Compare_Proc compare_proc)
{
    if (view.data == NULL || view.count == 0) return NULL;

    usize low = 0;
    usize high = view.count;
    const byte *base = (const byte*)view.data;

    while (low < high)
    {
        usize mid = low + (high - low) / 2;
        const void *item = base + (mid * view.stride);

        int compare = compare_proc(target, item);
        if (compare < 0) { high = mid; continue; }
        if (compare > 0) { low = mid + 1; continue; }

        return (void*)item;
    }

    return NULL;
}
#endif // MECH_IMPL

#define $find_binary_search(T, view, target, compare_proc) \
    (T*)find_binary_search_impl((view), (const void*)(target), (compare_proc))

#endif // MECH_FIND_H
