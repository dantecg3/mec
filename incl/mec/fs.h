#ifndef MEC_FS_H
#define MEC_FS_H

#include "base.h"
#include "str.h"
#include "context.h"

#define FS_MAX_PATH 4096

typedef STR_View FS_Path;
typedef STR_Buffer FS_Path_Buffer;

typedef enum {
    FS_EXT_UNKNOWN = 0x00,

    FS_EXT_TXT     = 0x01,
    FS_EXT_JSON    = 0x02,
    FS_EXT_YAML    = 0x03,
    FS_EXT_XML     = 0x04,
    FS_EXT_CSV     = 0x05,

    FS_EXT_LUA     = 0x10,
    FS_EXT_PY      = 0x11,
    FS_EXT_DYNLIB  = 0x12, // .so .dylib .dynlib
    FS_EXT_DLL     = 0x13, // .dll

    FS_EXT_GLSL    = 0x20,
    FS_EXT_HLSL    = 0x21,
    FS_EXT_VERT    = 0x22,
    FS_EXT_FRAG    = 0x23,
    FS_EXT_SPIRV   = 0x24, // .spv

    FS_EXT_BMP     = 0x30,
    FS_EXT_PNG     = 0x31,
    FS_EXT_JPG     = 0x32,
    FS_EXT_JPEG    = 0x33,
    FS_EXT_TGA     = 0x34,
    FS_EXT_HDR     = 0x35,

    FS_EXT_OBJ     = 0x40,
    FS_EXT_GLTF    = 0x41,
    FS_EXT_GLB     = 0x42,
    FS_EXT_FBX     = 0x43,

    FS_EXT_WAV     = 0x50,
    FS_EXT_OGG     = 0x51,
    FS_EXT_MP3     = 0x52,
} FS_Extension;

#define FS_EXTS_CAPACITY 8
typedef struct {
    usize size;
    FS_Extension ext;
    char exts[FS_EXTS_CAPACITY];
} FS_Metadata;

typedef struct {
    byte *buffer;
    FS_Path_Buffer path;
    FS_Metadata metadata;
    MEM_Allocator alloc;
} FS_File;

FS_Path_Buffer fs_exec_path(MEM_Allocator alloc);
FS_Path_Buffer fs_exec_directory_path(MEM_Allocator alloc);

FS_Path fs_path(FS_Path_Buffer buffer);
FS_Path fs_path_trim_trailing_sep(FS_Path path);
FS_Path fs_path_parent(FS_Path path);
FS_Path fs_path_filename(FS_Path path);
FS_Path fs_path_extension(FS_Path path);
FS_Path fs_path_stem(FS_Path path);
bool fs_path_is_absolute(FS_Path path);

FS_Path_Buffer fs_path_join(FS_Path a, FS_Path b, MEM_Allocator alloc);
FS_Path_Buffer fs_path_append(FS_Path path, const char *literal, MEM_Allocator alloc);

FS_File fs_load_file(FS_Path path, MEM_Allocator alloc);
void fs_free_file(FS_File *pfile);

#ifdef MEC_IMPL
#if defined(WINDOWS__)
#include <windows.h>
#elif defined(MACOS__)
#include <mach-o/dyld.h>
#include <unistd.h>
#elif defined(LINUX__)
#include <unistd.h>
#endif

#include <errno.h>

#if defined(WINDOWS__)
    #define $fs_is_sep(cchar) ((cchar) == '/' || (cchar) == '\\')
#else
    #define $fs_is_sep(cchar) ((cchar) == '/')
#endif

static usize fs_proc_path_raw(char out[FS_MAX_PATH])
{
#if defined(WINDOWS__)
    DWORD len = GetModuleFileNameA(NULL, out, FS_MAX_PATH);
    if (len == 0 || len >= FS_MAX_PATH) return 0;
    return (usize)len;
#elif defined(MACOS__)
    char tmp[FS_MAX_PATH];
    uint32 size = sizeof(tmp);
    if (_NSGetExecutablePath(tmp, &size) != 0) return 0;

    if (realpath(tmp, out) == NULL)
    {
        usize len = strlen(tmp);
        memcpy(out, tmp, len + 1);
        return len;
    }
    return strlen(out);
#elif defined(LINUX__)
    ssize_t len = readlink("/proc/self/exe", out, FS_MAX_PATH - 1);
    if (len <= 0) return 0;
    out[len] = '\0';
    return (usize)len;
#else
    #error "Unsupported platform for fs_proc_path_raw"
#endif
}

FS_Path_Buffer fs_exec_path(MEM_Allocator alloc)
{
    char buffer[FS_MAX_PATH];
    usize len = fs_proc_path_raw(buffer);
    if (len == 0) return NULL;
    return str_from_view(str_view_raw(buffer, len), alloc);
}

FS_Path_Buffer fs_exec_directory_path(MEM_Allocator alloc)
{
    char buffer[FS_MAX_PATH];
    usize len = fs_proc_path_raw(buffer);
    if (len == 0) return NULL;
    return str_from_view(fs_path_parent(str_view_raw(buffer, len)), alloc);
}

FS_Path fs_path(FS_Path_Buffer buffer)
{
    return str_view_buffer(buffer);
}

static usize fs_path_last_sep(FS_Path path)
{
    for (usize i = path.size; i > 0; --i)
    {
        if ($fs_is_sep(path.data[i - 1])) return i - 1;
    }
    return STR_NOT_FOUND;
}

FS_Path fs_path_trim_trailing_sep(FS_Path path)
{
    while (path.size > 1 && $fs_is_sep(path.data[path.size - 1])) path.size--;
    return path;
}

FS_Path fs_path_parent(FS_Path path)
{
    path = fs_path_trim_trailing_sep(path);
    usize sep = fs_path_last_sep(path);
    if (sep == STR_NOT_FOUND) return str_view_raw(path.data, 0);
    if (sep == 0) return str_view_raw(path.data, 1);
    return fs_path_trim_trailing_sep(str_view_raw(path.data, sep));
}

FS_Path fs_path_filename(FS_Path path)
{
    path = fs_path_trim_trailing_sep(path);
    usize sep = fs_path_last_sep(path);
    if (sep == STR_NOT_FOUND) return path;
    return str_view_range(path, $range(sep + 1, path.size));
}

FS_Path fs_path_extension(FS_Path path)
{
    FS_Path name = fs_path_filename(path);
    usize dot = str_view_find_char_from_top(name, '.');
    if (dot == STR_NOT_FOUND || dot == 0) return str_view_raw(name.data + name.size, 0);
    return str_view_range(name, $range(dot, name.size));
}

FS_Path fs_path_stem(FS_Path path)
{
    FS_Path name = fs_path_filename(path);
    name.size -= fs_path_extension(name).size;
    return name;
}

bool fs_path_is_absolute(FS_Path path)
{
    if (path.size > 0 && $fs_is_sep(path.data[0])) return true;
#if defined(WINDOWS__)
    if (path.size >= 3 && isalpha((unsigned char)path.data[0]) &&
        path.data[1] == ':' && $fs_is_sep(path.data[2])) return true;
#endif
    return false;
}

FS_Path_Buffer fs_path_join(FS_Path a, FS_Path b, MEM_Allocator alloc)
{
    while (b.size > 0 && $fs_is_sep(b.data[0]))
    {
        b.data++;
        b.size--;
    }

    FS_Path_Buffer buffer = str_from_view(a, alloc);
    if (buffer == NULL) return NULL;

    bool need_sep = a.size > 0 && !$fs_is_sep(a.data[a.size - 1]);
    if ((need_sep && !str_append(&buffer, "/")) || !str_append_view(&buffer, b))
    {
        str_free(&buffer);
        return NULL;
    }
    return buffer;
}

FS_Path_Buffer fs_path_append(FS_Path path, const char *literal, MEM_Allocator alloc)
{
    return fs_path_join(path, str_view(literal), alloc);
}

static const struct { const char *ext; FS_Extension id; } FS_EXT_MAP[] = {
    {".txt",   FS_EXT_TXT},   {".json",  FS_EXT_JSON},  {".yaml", FS_EXT_YAML},
    {".xml",   FS_EXT_XML},   {".csv",   FS_EXT_CSV},

    {".lua",   FS_EXT_LUA},   {".py",    FS_EXT_PY},
    {".so",    FS_EXT_DYNLIB},{".dylib", FS_EXT_DYNLIB},{".dynlib", FS_EXT_DYNLIB},
    {".dll",   FS_EXT_DLL},

    {".glsl",  FS_EXT_GLSL},  {".hlsl",  FS_EXT_HLSL},  {".vert", FS_EXT_VERT},
    {".frag",  FS_EXT_FRAG},  {".spv",   FS_EXT_SPIRV},

    {".bmp",   FS_EXT_BMP},   {".png",   FS_EXT_PNG},   {".jpg",  FS_EXT_JPG},
    {".jpeg",  FS_EXT_JPEG},  {".tga",   FS_EXT_TGA},   {".hdr",  FS_EXT_HDR},

    {".obj",   FS_EXT_OBJ},   {".gltf",  FS_EXT_GLTF},  {".glb",  FS_EXT_GLB},
    {".fbx",   FS_EXT_FBX},

    {".wav",   FS_EXT_WAV},   {".ogg",   FS_EXT_OGG},   {".mp3",  FS_EXT_MP3},
};

static inline FS_Extension fs_parse_extension(FS_Path path, char out_exts[FS_EXTS_CAPACITY])
{
    FS_Path ext = fs_path_extension(path);

    usize len = ext.size < (FS_EXTS_CAPACITY - 1) ? ext.size : (FS_EXTS_CAPACITY - 1);
    if (len > 0) memcpy(out_exts, ext.data, len);
    out_exts[len] = '\0';

    for (usize i = 0; i < sizeof(FS_EXT_MAP) / sizeof(FS_EXT_MAP[0]); ++i)
    {
        if (str_view_equal_literal(ext, FS_EXT_MAP[i].ext)) return FS_EXT_MAP[i].id;
    }

    return FS_EXT_UNKNOWN;
}

FS_File fs_load_file(FS_Path path, MEM_Allocator alloc)
{
    byte temp[2048];
    MEM_Arena temp_arena = {0};
    mem_arena_init(&temp_arena, mem_slice(temp, 2048), MEM_DEFAULT_ALIGNMENT);

    STR_Cstr cpath = str_cstr(path, mem_arena_allocator(&temp_arena));
    if (cpath == NULL)
    {
        $error("Failed to allocate temporary C-string for path: " SV_FMT, $sv_arg(path));
        return (FS_File){0};
    }

    FILE *f = fopen(cpath, "rb");
    if (f == NULL)
    {
        $error("Failed to open file '%s': %s (path: " SV_FMT ")", cpath, strerror(errno), $sv_arg(path));
        return (FS_File){0};
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0)
    {
        $error("Failed to determine size of file (invalid file size): " SV_FMT, $sv_arg(path));
        fclose(f);
        return (FS_File){0};
    }

    byte *buffer = $mem_alloc_bytes((usize)size + 1, alloc);
    if (buffer == NULL)
    {
        $error("Out of memory while allocating %ld bytes for file: " SV_FMT, size, $sv_arg(path));
        fclose(f);
        return (FS_File){0};
    }

    usize read_bytes = fread(buffer, 1, (usize)size, f);
    fclose(f);

    if (read_bytes != (usize)size)
    {
        $error("Failed to read file completely: expected %ld bytes, got %zu bytes for " SV_FMT, size, read_bytes, $sv_arg(path));
        $mem_free(&buffer, alloc);
        return (FS_File){0};
    }
    buffer[size] = '\0';

    FS_Path_Buffer path_buffer = str_from_view(path, alloc);
    if (path_buffer == NULL)
    {
        $error("Out of memory while copying path: " SV_FMT, $sv_arg(path));
        $mem_free(&buffer, alloc);
        return (FS_File){0};
    }

    FS_File file = (FS_File) {
        .buffer = buffer,
        .path = path_buffer,
        .metadata = (FS_Metadata) {
            .size = (usize)size,
        },
        .alloc = alloc
    };

    file.metadata.ext = fs_parse_extension(path, file.metadata.exts);
    return file;
}

void fs_free_file(FS_File *pfile)
{
    if (pfile == NULL) return;
    $mem_free(&pfile->buffer, pfile->alloc);
    str_free(&pfile->path);
    *pfile = (FS_File){0};
}
#endif // MEC_IMPL
#endif // MEC_FS_H
