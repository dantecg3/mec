#ifndef MEC_FS_H
#define MEC_FS_H

#include "base.h"
#include "context.h"
#include "sys.h"
#include "str.h"

typedef STR_View       FS_Path;
typedef STR_Buffer     FS_Path_Buffer;
typedef uint8          FS_Extension;
typedef struct FS_File FS_File;

FS_Path  fs_path (const char *path);
FS_Path  fs_path_from_buffer (FS_Path_Buffer buffer);
FS_Path  fs_path_parent (FS_Path path);
FS_Path  fs_path_filename (FS_Path path);
STR_View fs_path_extension (FS_Path path);
FS_Path  fs_path_stem (FS_Path path);
bool     fs_path_is_absolute (FS_Path path);

FS_Path_Buffer fs_path_buffer_new (FS_Path path, MEM_Allocator alloc);
FS_Path_Buffer fs_path_join (FS_Path a, FS_Path b, MEM_Allocator alloc);
void           fs_path_buffer_free (FS_Path_Buffer *p_buffer);
void           fs_path_append (FS_Path_Buffer *p_buffer, FS_Path path);

FS_Path_Buffer fs_proc_path (MEM_Allocator alloc);

enum
{
  FS_EXT_UNKNOWN = 0x00,

  FS_EXT_TXT  = 0x01,
  FS_EXT_JSON = 0x02,
  FS_EXT_YAML = 0x03,
  FS_EXT_XML  = 0x04,
  FS_EXT_CSV  = 0x05,

  FS_EXT_LUA    = 0x10,
  FS_EXT_PY     = 0x11,
  FS_EXT_DYNLIB = 0x12, // .so .dylib .dynlib
  FS_EXT_DLL    = 0x13, // .dll

  FS_EXT_GLSL  = 0x20,
  FS_EXT_HLSL  = 0x21,
  FS_EXT_VERT  = 0x22,
  FS_EXT_FRAG  = 0x23,
  FS_EXT_SPIRV = 0x24, // .spv

  FS_EXT_BMP  = 0x30,
  FS_EXT_PNG  = 0x31,
  FS_EXT_JPG  = 0x32,
  FS_EXT_JPEG = 0x33,
  FS_EXT_TGA  = 0x34,
  FS_EXT_HDR  = 0x35,

  FS_EXT_OBJ  = 0x40,
  FS_EXT_GLTF = 0x41,
  FS_EXT_GLB  = 0x42,
  FS_EXT_FBX  = 0x43,

  FS_EXT_WAV = 0x50,
  FS_EXT_OGG = 0x51,
  FS_EXT_MP3 = 0x52,
};

struct FS_File
{
  byte *buffer;
  struct
  {
    usize        size;
    FS_Extension extension;
  } metadata;
  MEM_Allocator alloc;
};

FS_File fs_load_file (FS_Path path, MEM_Allocator alloc);
void    fs_free_file (FS_File *p_file);

#ifdef MEC_IMPL
static inline const char *fs_separator ()
{
#ifdef WINDOWS__
  return "\\";
#elif defined(LINUX__) || defined(MACOS)
  return "/";
#endif
  return "\0";
}

static inline FS_Extension fs_map_extension (STR_View extension)
{
  if (str_view_equal (extension, str_view (".txt")))
    return FS_EXT_TXT;
  if (str_view_equal (extension, str_view (".json")))
    return FS_EXT_JSON;
  if (str_view_equal (extension, str_view (".yaml")) || str_view_equal (extension, str_view (".yml")))
    return FS_EXT_YAML;
  if (str_view_equal (extension, str_view (".xml")))
    return FS_EXT_XML;
  if (str_view_equal (extension, str_view (".csv")))
    return FS_EXT_CSV;

  if (str_view_equal (extension, str_view (".lua")))
    return FS_EXT_LUA;
  if (str_view_equal (extension, str_view (".py")))
    return FS_EXT_PY;
  if (str_view_equal (extension, str_view (".so")) || str_view_equal (extension, str_view (".dylib")) || str_view_equal (extension, str_view (".dynlib")))
    return FS_EXT_DYNLIB;
  if (str_view_equal (extension, str_view (".dll")))
    return FS_EXT_DLL;

  if (str_view_equal (extension, str_view (".glsl")))
    return FS_EXT_GLSL;
  if (str_view_equal (extension, str_view (".hlsl")))
    return FS_EXT_HLSL;
  if (str_view_equal (extension, str_view (".vert")))
    return FS_EXT_VERT;
  if (str_view_equal (extension, str_view (".frag")))
    return FS_EXT_FRAG;
  if (str_view_equal (extension, str_view (".spv")))
    return FS_EXT_SPIRV;

  if (str_view_equal (extension, str_view (".bmp")))
    return FS_EXT_BMP;
  if (str_view_equal (extension, str_view (".png")))
    return FS_EXT_PNG;
  if (str_view_equal (extension, str_view (".jpg")))
    return FS_EXT_JPG;
  if (str_view_equal (extension, str_view (".jpeg")))
    return FS_EXT_JPEG;
  if (str_view_equal (extension, str_view (".tga")))
    return FS_EXT_TGA;
  if (str_view_equal (extension, str_view (".hdr")))
    return FS_EXT_HDR;

  if (str_view_equal (extension, str_view (".obj")))
    return FS_EXT_OBJ;
  if (str_view_equal (extension, str_view (".gltf")))
    return FS_EXT_GLTF;
  if (str_view_equal (extension, str_view (".glb")))
    return FS_EXT_GLB;
  if (str_view_equal (extension, str_view (".fbx")))
    return FS_EXT_FBX;

  if (str_view_equal (extension, str_view (".wav")))
    return FS_EXT_WAV;
  if (str_view_equal (extension, str_view (".ogg")))
    return FS_EXT_OGG;
  if (str_view_equal (extension, str_view (".mp3")))
    return FS_EXT_MP3;

  return FS_EXT_UNKNOWN;
}

FS_Path fs_path (const char *path)
{
  return str_view (path);
}

FS_Path fs_path_from_buffer (FS_Path_Buffer buffer)
{
  return str_view_from_buffer (buffer);
}

FS_Path fs_path_parent (FS_Path path)
{
  usize separator = str_view_find_rune_from_top (path, str_rune (fs_separator ()));
  if (separator == STR_NOT_FOUND)
    return path;

  return str_view_slice (path, range_to (separator));
}

FS_Path fs_path_filename (FS_Path path)
{
  usize separator = str_view_find_rune_from_top (path, str_rune (fs_separator ()));
  if (separator == STR_NOT_FOUND)
    return path;

  return str_view_slice (path, range (separator + 1, path.length));
}

STR_View fs_path_extension (FS_Path path)
{
  usize point = str_view_find_rune_from_top (path, str_rune ("."));
  if (point == STR_NOT_FOUND)
    return str_view ("");

  return str_view_slice (path, range (point, path.length));
}

FS_Path fs_path_stem (FS_Path path)
{
  FS_Path filename = fs_path_filename (path);
  usize   point    = str_view_find_rune_from_top (filename, str_rune ("."));
  if (point == STR_NOT_FOUND)
    return filename;

  return str_view_slice (filename, range_to (point));
}

bool fs_path_is_absolute (FS_Path path)
{
  if (path.length == 0)
    return false;
#ifdef WINDOWS__
  if (path.length >= 2 && path.ptr[1] == ':')
    return true;
  return path.data[0] == '/' || path.ptr[0] == '\\';
#elif defined(LINUX__) || defined(MACOS)
  return path.data[0] == '/';
#endif
  return false;
}

FS_Path_Buffer fs_path_buffer_new (FS_Path path, MEM_Allocator alloc)
{
  return str_new (path, alloc);
}

FS_Path_Buffer fs_path_join (FS_Path a, FS_Path b, MEM_Allocator alloc)
{
  FS_Path_Buffer buffer = fs_path_buffer_new (a, alloc);
  str_append (&buffer, str_view (fs_separator ()));
  str_append (&buffer, b);
  return buffer;
}

void fs_path_buffer_free (FS_Path_Buffer *p_buffer)
{
  str_free (p_buffer);
}

void fs_path_append (FS_Path_Buffer *p_buffer, FS_Path path)
{
  str_append (p_buffer, str_view (fs_separator ()));
  str_append (p_buffer, path);
}

FS_Path_Buffer fs_proc_path (MEM_Allocator alloc)
{
  char buffer[SYS_MAX_PATH_SIZE];
  sys_proc_path (buffer);
  return str_new (str_view (buffer), alloc);
}

FS_File fs_load_file (FS_Path path, MEM_Allocator alloc)
{
  $mem_scope_init (1 * MEM_KB, &s_context.temp_allocator);

  STR_C path_c = str_c_new (path, s_context.temp_allocator);
  if (path_c == NULL)
    {
      $error_stack_push (error (ERROR_STACK_OVERFLOW, "failed to allocate path_c in fs_load_file"));
      return (FS_File){ 0 };
    }

  FILE *f = fopen (path_c, "rb");
  if (f == NULL)
    {
      $error_stack_push (error (ERROR_IO_DEVICE, "failed to open a file"));
      return (FS_File){ 0 };
    }

  fseek (f, 0, SEEK_END);
  long size = ftell (f);
  fseek (f, 0, SEEK_SET);

  if (size < 0)
    {
      $error_stack_push (error (ERROR_FILE_CORRUPTED, "failed to determine size of file"));
      fclose (f);
      return (FS_File){ 0 };
    }

  byte *buffer = mem_alloc_bytes ((usize)size + 1, alloc);
  if (buffer == NULL)
    {
      $error_stack_push (error (ERROR_OUT_OF_MEMORY, "failed to allocate file buffer"));
      fclose (f);
      return (FS_File){ 0 };
    }

  usize readed = fread (buffer, 1, (usize)size, f);
  fclose (f);

  if (readed != (usize)size)
    {
      $error_stack_push (error (ERROR_FILE_CORRUPTED, "failed to read the whole file"));
      $mem_free (&buffer, alloc);
      return (FS_File){ 0 };
    }

  buffer[size] = '\0';

  return (FS_File){
    .buffer   = buffer,
    .metadata = {
        .size      = (usize)size,
        .extension = fs_map_extension (fs_path_extension (path)) },
    .alloc = alloc
  };
}

void fs_free_file (FS_File *p_file)
{
  $assert (p_file != NULL, "");
  $mem_free (&p_file->buffer, p_file->alloc);
}

#endif // MEC_IMPL
#endif // MEC_FS_H
