#ifndef MEC_STR_H
#define MEC_STR_H

#include "base.h"
#include "range.h"
#include "mem.h"
#include "dynarr.h"

typedef struct {
    const char *data;
    usize size;
} STR_View;

typedef char* STR_Buffer;
typedef char* STR_Cstr;

STR_View str_view(const char *literal);
STR_View str_view_raw(const char *buffer, usize size);
STR_View str_view_buffer(STR_Buffer buffer);
STR_View str_view_range(STR_View view, Range range);
bool str_view_equal(STR_View a, STR_View b);
bool str_view_equal_literal(STR_View a, const char *literal);
bool str_view_starts_with(STR_View view, STR_View prefix);
bool str_view_ends_with(STR_View view, STR_View suffix);
usize str_view_find_char(STR_View view, char cchar);
usize str_view_find_char_from_top(STR_View view, char cchar);
STR_View str_view_trim_left(STR_View view);
STR_View str_view_trim_right(STR_View view);
STR_View str_view_trim(STR_View view);
STR_View str_view_chop_by_char(STR_View *view, char cchar);

#define SV_FMT "%.*s"
#define $sv_arg(sv) (int)(sv).size, (const char*)(sv).data
#define STR_NOT_FOUND ((usize)-1)

STR_Buffer str_new(const char *literal, MEM_Allocator alloc);
STR_Buffer str_from_view(STR_View view, MEM_Allocator alloc);
void str_free(STR_Buffer *pstr);
bool str_append(STR_Buffer *pstr, const char* literal);
bool str_append_view(STR_Buffer *pstr, STR_View view);
usize str_len(STR_Buffer str);
usize str_capacity(STR_Buffer str);

STR_Cstr str_cstr(STR_View view, MEM_Allocator alloc);
void str_free_cstr(STR_Cstr *pcstr);

#ifdef MEC_IMPL
STR_View str_view(const char *literal)
{
    return (STR_View) {
        .data = literal,
        .size = strlen(literal)
    };
}

STR_View str_view_raw(const char *buffer, usize size)
{
    return (STR_View) {
        .data = buffer,
        .size = size
    };
}

STR_View str_view_buffer(STR_Buffer buffer)
{
    if (buffer == NULL) return (STR_View) {0};
    return (STR_View) {
        .data = buffer,
        .size = dynarr_count(buffer)
    };
}

STR_View str_view_range(STR_View view, Range range)
{
    range = range_intersect(range, $range(0, view.size));
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

bool str_view_equal_literal(STR_View a, const char *literal)
{
    return str_view_equal(a, str_view(literal));
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

usize str_view_find_char_from_top(STR_View view, char cchar)
{
    for (usize i = view.size - 1; i > 0; --i) if (view.data[i] == cchar) return i;
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

STR_Buffer str_new(const char *literal, MEM_Allocator alloc)
{
    return str_from_view(str_view(literal), alloc);
}

STR_Buffer str_from_view(STR_View view, MEM_Allocator alloc)
{
    STR_Buffer str = (STR_Buffer)$dynarr_new(char, alloc);
    str_append_view(&str, view);
    return str;
}

void str_free(STR_Buffer *pstr)
{
    $dynarr_free(pstr);
}

bool str_append(STR_Buffer *pstr, const char *literal)
{
    return str_append_view(pstr, str_view(literal));
}

bool str_append_view(STR_Buffer *pstr, STR_View view)
{
    // TODO change for $dynarr_append_view(dynarr, MEM_View)
    for (usize i = 0; i < view.size; ++i)
    {
        char cchar = view.data[i];
        if(!$dynarr_append(pstr, &cchar)) return false;
    }
    return true;
}

usize str_len(STR_Buffer str)
{
    return dynarr_count(str);
}

usize str_capacity(STR_Buffer str)
{
    return dynarr_capacity(str);
}

typedef struct {
    MEM_Allocator alloc;
} STR_Cstr_Header;

STR_Cstr str_cstr(STR_View view, MEM_Allocator alloc)
{
    if (view.data == NULL || view.size == 0) return NULL;

    usize size = sizeof(STR_Cstr_Header) + view.size + 1;
    STR_Cstr_Header *header = (STR_Cstr_Header*)$mem_alloc_bytes(size, alloc);

    char *cstr = (char*)(header + 1);
    memcpy(cstr, view.data, view.size);
    cstr[view.size] = '\0';

    *header = (STR_Cstr_Header) {
        .alloc = alloc
    };

    return (STR_Cstr)cstr;
}

void str_free_cstr(STR_Cstr *pcstr)
{
    if (*pcstr == NULL) return;
    STR_Cstr_Header *header = ((STR_Cstr_Header*)*pcstr) - 1;
    MEM_Allocator alloc = header->alloc;
    $mem_free(&header, alloc);
    *pcstr = NULL;
}

#endif // MEC_IMPL
#endif // MEC_STR_H
