#ifndef MEC_DYNLIB_H
#define MEC_DYNLIB_H

#include "base.h"
#include "fs.h"

#ifdef WINDOWS__
    #include <windows.h>
    typedef HMODULE DYNLIB_Handle;
#elif defined(LINUX__) ||defined(MAXOS__)
    #include <dlfcn.h>
    typedef void* DYNLIB_Handle;
#else
    #error "unsupported platform"
#endif

DYNLIB_Handle dynlib_load(FS_Path path);
void *dynlib_symbol_impl(DYNLIB_Handle dynlib, const char *name);
void dynlib_close(DYNLIB_Handle dynlib);
const char *dynlib_error(void);

#define $dynlib_symbol(type, dynlib, name) ((union { void *obj_ptr; type func_ptr; }) { .obj_ptr = dynlib_symbol_impl((dynlib), (name)) }).func_ptr

#ifdef MEC_IMPL
DYNLIB_Handle dynlib_load(FS_Path path)
{
    byte buffer[FS_MAX_PATH];
    MEM_Arena arena = {0};
    mem_arena_init(&arena, mem_slice(buffer, FS_MAX_PATH), MEM_DEFAULT_ALIGNMENT);
    STR_Cstr cpath = str_cstr(path, mem_arena_allocator(&arena));
#ifdef WINDOWS__
    HMODULE handle = LoadLibraryA(cpath);
    return handle
#elif defined(LINUX__) ||defined(MAXOS__)
    void* handle = dlopen(cpath, RTLD_LAZY | RTLD_LOCAL);
    return handle;
#else
    return NULL;
#endif
}

void *dynlib_symbol_impl(DYNLIB_Handle dynlib, const char *name)
{
#ifdef WINDOWS__
    return (void*)GetProcAddress((HMODULE)dynlib, name);
#elif defined(LINUX__) || defined(MACOS__)
    return dlsym(dynlib, name);
#else
    return NULL;
#endif
}

void dynlib_close(DYNLIB_Handle dynlib)
{
#if defined(_WIN32) || defined(_WIN64)
    FreeLibrary((HMODULE)dynlib);
#else
    dlclose(dynlib);
#endif
}

const char *dynlib_error(void)
{
#ifdef WINDOWS__
    // TODO expand with GetLastError()
    return "Windows dynamic loading error!";
#elif defined(LINUX__) || defined(MACOS__)
    return dlerror();
#else
    return NULL;
#endif
}
#endif // MEC_IMPL
#endif // MEC_DYNLIB_H
