#ifndef MECH_SORT_H
#define MECH_SORT_H

#include "base.h"
#include "mem.h"

typedef int (*SORT_Compare_Proc)(const void *target, const void *item);

void *sort(MEM_View view, SORT_Compare_Proc compare, MEM_Allocator alloc);

#define $sort(T, view, compare, alloc) (T*)sort(view, compare, alloc)

#ifdef MECH_IMPL
static byte *sort_item(MEM_View view, usize i)
{
    return (byte*)view.data + i * view.stride;
}

static void sort_swap(MEM_View view, usize i, usize j)
{
    byte *a = sort_item(view, i);
    byte *b = sort_item(view, j);

    for (usize n = view.stride; n--; )
    {
        byte t = *a; *a++ = *b; *b++ = t;
    }
}

static void sort_sift_down(MEM_View view, usize root, usize count, SORT_Compare_Proc compare)
{
    for (;;)
    {
        usize child = root * 2 + 1;
        if (child >= count) return;

        byte *parent  = sort_item(view, root);
        byte *left    = sort_item(view, child);
        byte *right   = sort_item(view, child + 1);
        byte *greater = left;

        if (child + 1 < count && compare(left, right) < 0) {
            greater = right;
            child++;
        }

        if (compare(parent, greater) >= 0) return;

        sort_swap(view, root, child);
        root = child;
    }
}

static void sort_heapify(MEM_View view, SORT_Compare_Proc compare)
{
    for (usize i = view.count / 2; i-- > 0;)
    {
        sort_sift_down(view, i, view.count, compare);
    }
}

static void sort_in_place(MEM_View view, SORT_Compare_Proc compare)
{
    sort_heapify(view, compare);

    for (usize end = view.count; end-- > 1;)
    {
        sort_swap(view, 0, end);
        sort_sift_down(view, 0, end, compare);
    }
}

void *sort(MEM_View view, SORT_Compare_Proc compare, MEM_Allocator alloc)
{
    usize size = view.count * view.stride;

    byte *sorted = $mem_new_array(byte, size, alloc);
    if (sorted == NULL) return NULL; // Out_Of_Memory

    memcpy(sorted, view.data, size);

    MEM_View copy = view;
    copy.data = sorted;

    sort_in_place(copy, compare);

    return sorted;
}
#endif // MECH_IMPL
#endif // MECH_SORT_H
