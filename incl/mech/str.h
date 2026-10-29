#ifndef MECH_STR_H
#define MECH_STR_H

#include "base.h"

typedef struct {
    const char *data;
    usize size;
} STR_View;

STR_View str_view(const char *literal);
STR_View str_view_range(STR_View view, Range range);
bool str_view_equal(STR_View a, STR_View b);
bool str_view_starts_with(STR_View view, STR_View prefix);
bool str_view_ends_with(STR_View view, STR_View suffix);
usize str_view_find_char(STR_View view, char cchar);
STR_View str_view_trim_left(STR_View view);
STR_View str_view_trim_right(STR_View view);
STR_View str_view_trim(STR_View view);
STR_View str_view_chop_by_char(STR_View *view, char cchar);

#define SV_FMT "%.*s"
#define SV_ARG(sv) (int)(sv).size, (const char*)(sv).data
#define STR_NOT_FOUND ((usize)-1)

#ifdef MECH_IMPL
STR_View str_view(const char *literal)
{
    return (STR_View) {
        .data = literal,
        .size = strlen(literal)
    };
}

STR_View str_view_range(STR_View view, Range range)
{
    range = range_intersect(range, RANGE(0, view.size));
    return (STR_View) {
        .data = view.data + range.lo,
        .size = range_len(range)
    };
}

bool str_view_equal(STR_View a, STR_View b)
{
    return
        a.size == b.size &&
        (a.size == 0 || memcmp(a.data, b.data, a.size) == 0);
}

bool str_view_starts_with(STR_View view, STR_View prefix)
{
    return
        view.size >= prefix.size &&
        memcmp(view.data, prefix.data, prefix.size) == 0;
}

bool str_view_ends_with(STR_View view, STR_View suffix)
{
    return
        view.size >= suffix.size &&
        memcmp(view.data + view.size - suffix.size, suffix.data, suffix.size) == 0;
}

usize str_view_find_char(STR_View view, char cchar)
{
    for (usize i = 0; i < view.size; ++i) if (view.data[i] == cchar) return i;
    return STR_NOT_FOUND;
}

STR_View str_view_trim_left(STR_View view)
{
    while (view.size > 0 && isspace((unsigned char)view.data[0]))
    {
        view.data++;
        view.size--;
    }
    return view;
}

STR_View str_view_trim_right(STR_View view)
{
    while (view.size > 0 && isspace((unsigned char)view.data[view.size - 1]))
    {
        view.size--;
    }
    return view;
}

STR_View str_view_trim(STR_View view)
{
    return str_view_trim_right(str_view_trim_left(view));
}

STR_View str_view_chop_by_char(STR_View *view, char cchar)
{
    usize i = str_view_find_char(*view, cchar);
    if (i == STR_NOT_FOUND) {
        STR_View all = *view;
        view->data += view->size;
        view->size = 0;
        return all;
    }

    STR_View head = (STR_View) {
        .data = view->data,
        .size = i
    };

    view->data += i + 1;
    view->size -= i + 1;

    return head;
}
#endif // MECH_IMPL
#endif // MECH_STR_H
