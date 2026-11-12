#ifndef MEC_STR_H
#define MEC_STR_H

#include "base.h"
#include "context.h"
#include "mem.h"
#include "range.h"

typedef uint32          STR_Rune;
typedef struct STR_View STR_View;
typedef uint8          *STR_Buffer;
typedef struct STR_Iter STR_Iter;
typedef char           *STR_C;

#define SV_FMT        "%.*s"
#define SV_ARG(sv)    (int)(sv).length, (const char *)(sv).data
#define STR_NOT_FOUND ((usize) - 1)
#define STR_GROW_MUL  2

struct STR_View
{
  uint8 *data;
  usize  length;
};

struct STR_Iter
{
  STR_View view;
  usize    pos;
};

STR_Rune str_rune (const char *cstr);

STR_View str_view (const char *cstr);
STR_View str_view_from_raw (const uint8 *buffer, usize size);
STR_View str_view_from_buffer (STR_Buffer buffer);
STR_View str_view_slice (STR_View view, RANGE range);
bool     str_view_equal (STR_View a, STR_View b);
bool     str_view_starts_with (STR_View view, STR_View prefix);
bool     str_view_ends_with (STR_View a, STR_View suffix);
usize    str_view_find_rune (STR_View view, STR_Rune rune);
usize    str_view_find_rune_from_top (STR_View view, STR_Rune rune);
STR_View str_view_trim_right (STR_View view);
STR_View str_view_trim_left (STR_View view);
STR_View str_view_trim (STR_View view);

STR_Buffer str_new (STR_View view, MEM_Allocator alloc);
void       str_free (STR_Buffer *p_str);
void       str_append (STR_Buffer *p_str, STR_View view);
usize      str_rune_count (STR_Buffer str);
usize      str_rune_offset (STR_Buffer str, usize index);
usize      str_length (STR_Buffer str);
usize      str_capacity (STR_Buffer str);

STR_Iter str_iter (STR_Buffer str);
bool     str_iter_next (STR_Iter *iter, STR_Rune *rune);

STR_C str_c_new (STR_View view, MEM_Allocator alloc);
void  str_c_free (STR_C *p_str_c);

#ifdef MEC_IMPL
typedef struct STR_Header
{
  usize         length;
  usize         capacity;
  MEM_Allocator alloc;
} STR_Header;

static inline STR_Header *str_header (STR_Buffer str)
{
  return (STR_Header *)str - 1;
}

#define RUNE_MAX          0x10FFFF
#define RUNE_REPLACEMENT  0xFFFD
#define RUNE_SURROGATE_LO 0xD800
#define RUNE_SURROGATE_HI 0xDFFF

static inline bool rune_is_valid (STR_Rune rune)
{
  bool is_surrogate = rune >= RUNE_SURROGATE_LO && rune <= RUNE_SURROGATE_HI;
  return rune <= RUNE_MAX && !is_surrogate;
}

static inline bool utf8_is_continuation (uint8 byte)
{
  return (byte & 0xC0) == 0x80;
}

static inline int32 utf8_sequence_length (uint8 lead)
{
  if (lead <= 0x7F)
    return 1;
  if (lead >= 0xC2 && lead <= 0xDF)
    return 2;
  if (lead >= 0xE0 && lead <= 0xEF)
    return 3;
  if (lead >= 0xF0 && lead <= 0xF4)
    return 4;
  return 0;
}

static inline STR_Rune utf8_pack (const uint8 *bytes, int32 length)
{
  if (length == 1)
    return bytes[0];

  STR_Rune value = bytes[0] & (0xFF >> (length + 1));

  for (int32 i = 1; i < length; ++i)
    value = (value << 6) | (bytes[i] & 0x3F);

  return value;
}

static inline void utf8_unpack (STR_Rune rune, int32 length, uint8 *out)
{
  if (length == 1)
    {
      out[0] = (uint8)rune;
      return;
    }

  for (int32 i = length - 1; i > 0; --i)
    {
      out[i] = 0x80 | (rune & 0x3F);
      rune >>= 6;
    }

  out[0] = (uint8)(0xFF << (8 - length)) | (uint8)rune;
}

static inline int32 utf8_decode (const uint8 *bytes, usize count, STR_Rune *rune)
{
  *rune = RUNE_REPLACEMENT;
  if (count == 0)
    return 0;

  int32 length = utf8_sequence_length (bytes[0]);
  if (length == 0 || (usize)length > count)
    return 0;

  for (int32 i = 1; i < length; ++i)
    {
      if (!utf8_is_continuation (bytes[i]))
        return 0;
    }

  STR_Rune value = utf8_pack (bytes, length);

  static const STR_Rune smallest_for_length[5] = { 0, 0, 0x80, 0x800, 0x10000 };
  if (value < smallest_for_length[length])
    return 0;

  if (!rune_is_valid (value))
    return 0;

  *rune = value;
  return length;
}

static inline int32 utf8_encode (STR_Rune rune, uint8 out[4])
{
  if (!rune_is_valid (rune))
    return 0;

  int32 length = 4;
  if (rune <= 0x7F)
    length = 1;
  else if (rune <= 0x7FF)
    length = 2;
  else if (rune <= 0xFFFF)
    length = 3;

  utf8_unpack (rune, length, out);
  return length;
}

static inline bool utf8_is_valid (const uint8 *bytes, usize count)
{
  usize offset = 0;
  while (offset < count)
    {
      if (bytes[offset] < 0x80)
        {
          ++offset;
          continue;
        }

      STR_Rune rune;
      int32    used = utf8_decode (bytes + offset, count - offset, &rune);
      if (used == 0)
        return false;

      offset += (usize)used;
    }
  return true;
}

static inline bool rune_is_space (STR_Rune rune)
{
  return (rune >= 9 && rune <= 13) || rune == 0x20 || rune == 0x85 || rune == 0xA0
         || rune == 0x1680 || (rune >= 0x2000 && rune <= 0x200A)
         || rune == 0x2028 || rune == 0x2029 || rune == 0x202F
         || rune == 0x205F || rune == 0x3000;
}

static inline usize utf8_prev_start (const uint8 *bytes, usize end)
{
  usize start = end - 1;
  while (start > 0 && utf8_is_continuation (bytes[start]))
    --start;
  return start;
}

static inline bool utf8_is_boundary (STR_View view, usize index)
{
  if (index == view.length)
    return true;
  return index < view.length && !utf8_is_continuation (view.data[index]);
}

STR_Rune str_rune (const char *cstr)
{
  $assert (cstr != NULL, "");

  usize count = strlen (cstr);

  STR_Rune rune;
  int32    used = utf8_decode ((const uint8 *)cstr, count, &rune);

  $assert (used > 0 && (usize)used == count, "literal must be exactly one valid rune");
  return rune;
}

STR_View str_view (const char *cstr)
{
  $assert (cstr != NULL, "");

  usize length = strlen (cstr);
  $assert (utf8_is_valid ((const uint8 *)cstr, length), "invalid UTF-8");

  return (STR_View){
    .data   = (uint8 *)cstr,
    .length = length
  };
}

STR_View str_view_from_raw (const uint8 *buffer, usize size)
{
  $assert (buffer != NULL, "");
  $assert (utf8_is_valid (buffer, size), "invalid UTF-8");

  return (STR_View){
    .data   = (uint8 *)buffer,
    .length = size
  };
}

STR_View str_view_from_buffer (STR_Buffer buffer)
{
  $assert (buffer != NULL, "");
  return (STR_View){
    .data   = (uint8 *)buffer,
    .length = str_header (buffer)->length
  };
}

STR_View str_view_slice (STR_View view, RANGE range)
{
  $assert (view.data != NULL, "");
  $assert (range.low + range_length (range) <= view.length, "");

  usize end = range.low + range_length (range);
  $assert (utf8_is_boundary (view, range.low), "slice starts inside a rune");
  $assert (utf8_is_boundary (view, end), "slice ends inside a rune");

  return (STR_View){
    .data   = view.data + range.low,
    .length = range_length (range)
  };
}

bool str_view_equal (STR_View a, STR_View b)
{
  return a.length == b.length && (a.length == 0 || memcmp (a.data, b.data, a.length) == 0);
}

bool str_view_starts_with (STR_View view, STR_View prefix)
{
  return view.length >= prefix.length && (prefix.length == 0 || memcmp (view.data, prefix.data, prefix.length) == 0);
}

bool str_view_ends_with (STR_View view, STR_View suffix)
{
  return view.length >= suffix.length
         && (suffix.length == 0 || memcmp (view.data + view.length - suffix.length, suffix.data, suffix.length) == 0);
}

usize str_view_find_rune (STR_View view, STR_Rune rune)
{
  uint8 encoded[4];
  int32 size = utf8_encode (rune, encoded);
  $assert (size != 0, "invalid rune");

  if ((usize)size > view.length)
    return STR_NOT_FOUND;

  for (usize i = 0; i + size <= view.length; ++i)
    {
      if (memcmp (view.data + i, encoded, size) == 0)
        return i;
    }

  return STR_NOT_FOUND;
}

usize str_view_find_rune_from_top (STR_View view, STR_Rune rune)
{
  uint8 encoded[4];
  int32 size = utf8_encode (rune, encoded);
  $assert (size != 0, "invalid rune");

  if ((usize)size > view.length)
    return STR_NOT_FOUND;

  for (usize i = view.length - size + 1; i-- > 0;)
    {
      if (memcmp (view.data + i, encoded, size) == 0)
        return i;
    }

  return STR_NOT_FOUND;
}

STR_View str_view_trim_right (STR_View view)
{
  usize end = view.length;
  while (end > 0)
    {
      usize start = utf8_prev_start (view.data, end);

      STR_Rune rune;
      int32    used = utf8_decode (view.data + start, end - start, &rune);
      $assert (used > 0, "invalid UTF-8");

      if (!rune_is_space (rune))
        break;

      end = start;
    }

  return (STR_View){
    .data   = view.data,
    .length = end
  };
}

STR_View str_view_trim_left (STR_View view)
{
  usize start = 0;
  while (start < view.length)
    {
      STR_Rune rune;
      int32    used = utf8_decode (view.data + start, view.length - start, &rune);
      $assert (used > 0, "invalid UTF-8");

      if (!rune_is_space (rune))
        break;

      start += (usize)used;
    }

  return (STR_View){
    .data   = view.data + start,
    .length = view.length - start
  };
}

STR_View str_view_trim (STR_View view)
{
  return str_view_trim_right (str_view_trim_left (view));
}

STR_Buffer str_new (STR_View view, MEM_Allocator alloc)
{
  $assert (view.data != NULL, "");

  usize       size   = sizeof (STR_Header) + view.length;
  STR_Header *header = (STR_Header *)mem_alloc_bytes (size, alloc);

  if (header == NULL)
    {
      $error_stack_push (error (ERROR_OUT_OF_MEMORY, "failed to allocate a str buffer"));
      return NULL;
    }

  header->capacity = view.length;
  header->length   = view.length;
  header->alloc    = alloc;

  STR_Buffer buffer = (STR_Buffer)(header + 1);
  memcpy (buffer, view.data, view.length);
  return buffer;
}

void str_free (STR_Buffer *p_str)
{
  $assert (p_str != NULL && *p_str != NULL, "");

  STR_Header   *header = str_header (*p_str);
  MEM_Allocator alloc  = header->alloc;
  $mem_free (&header, alloc);
  *p_str = NULL;
}

void str_append (STR_Buffer *p_str, STR_View view)
{
  $assert (p_str != NULL && *p_str != NULL, "");
  $assert (view.data != NULL, "");

  STR_Header *header     = str_header (*p_str);
  usize       new_length = header->length + view.length;
  if (new_length > header->capacity)
    {
      usize new_capacity = header->capacity * STR_GROW_MUL;
      if (new_capacity < new_length)
        new_capacity = new_length;

      usize       new_size = sizeof (STR_Header) + new_capacity;
      STR_Header *grown    = (STR_Header *)mem_realloc_bytes ((byte *)header, new_size, header->alloc);

      if (grown == NULL)
        {
          $error_stack_push (error (ERROR_OUT_OF_MEMORY, "failed to realloc growing a str buffer"));
          return;
        }

      header           = grown;
      header->capacity = new_capacity;
      *p_str           = (STR_Buffer)(header + 1);
    }

  memcpy ((byte *)*p_str + header->length, view.data, view.length);
  header->length += view.length;
}

usize str_rune_count (STR_Buffer str)
{
  $assert (str != NULL, "");

  STR_View view  = str_view_from_buffer (str);
  usize    count = 0;

  for (usize i = 0; i < view.length; ++i)
    {
      if (!utf8_is_continuation (view.data[i]))
        ++count;
    }

  return count;
}

usize str_rune_offset (STR_Buffer str, usize index)
{
  $assert (str != NULL, "");

  STR_View view   = str_view_from_buffer (str);
  usize    offset = 0;

  while (index > 0)
    {
      $assert (offset < view.length, "rune index out of range");
      offset += (usize)utf8_sequence_length (view.data[offset]);
      --index;
    }

  return offset;
}

usize str_length (STR_Buffer str)
{
  $assert (str != NULL, "");
  return str_header (str)->length;
}

usize str_capacity (STR_Buffer str)
{
  $assert (str != NULL, "");
  return str_header (str)->capacity;
}

STR_Iter str_iter (STR_Buffer str)
{
  $assert (str != NULL, "");
  return (STR_Iter){
    .view = str_view_from_buffer (str),
    .pos  = 0
  };
}

bool str_iter_next (STR_Iter *iter, STR_Rune *rune)
{
  $assert (iter != NULL && rune != NULL, "");

  if (iter->pos >= iter->view.length)
    return false;

  int32 used = utf8_decode (iter->view.data + iter->pos, iter->view.length - iter->pos, rune);
  $assert (used > 0, "invalid UTF-8");

  iter->pos += (usize)used;
  return true;
}

STR_C str_c_new (STR_View view, MEM_Allocator alloc)
{
  $assert (view.data != NULL, "");
  $assert (view.length == 0 || memchr (view.data, 0, view.length) == NULL, "U+0000 cannot live in a C string");

  usize       size   = sizeof (STR_Header) + view.length + 1;
  STR_Header *header = (STR_Header *)mem_alloc_bytes (size, alloc);

  if (header == NULL)
    {
      $error_stack_push (error (ERROR_OUT_OF_MEMORY, "failed to allocate a C str"));
      return NULL;
    }

  header->length   = view.length;
  header->capacity = view.length;
  header->alloc    = alloc;

  char *c_str = (char *)(header + 1);
  memcpy (c_str, view.data, view.length);
  c_str[view.length] = '\0';

  return c_str;
}

void str_c_free (STR_C *p_str_c)
{
  $assert (p_str_c != NULL && *p_str_c != NULL, "");

  STR_Header   *header = str_header ((STR_Buffer)*p_str_c);
  MEM_Allocator alloc  = header->alloc;
  $mem_free (&header, alloc);
  *p_str_c = NULL;
}

#endif // MEC_IMPL
#endif // MEC_STR_H
