#ifndef DYNLYB
#define DYNLYB 1

#include <stdio.h>
#include <assert.h>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
typedef HMODULE dyn_handle_t;
#define DYN_LIBRARY(lib) lib ".dll"
#define RTLD_LAZY 0 // Use this as workaround
#define RTLD_GLOBAL 0
#else
#ifndef __USE_GNU
#define __USE_GNU // This is required for dlinfo and dladdr
#endif
#include <dlfcn.h>
typedef void* dyn_handle_t;
#ifdef __APPLE__
#define DYN_LIBRARY(lib) "lib" lib ".dylib"
#else
#define DYN_LIBRARY(lib) "lib" lib ".so"
#include <link.h>
#endif
#endif

#define DYN_LIBRARY_PATH(path, lib) path DYN_LIBRARY(lib)

static inline int dyn_open(char *lib, int flags, dyn_handle_t *handle) {
#if defined(WIN32) || defined(_WIN32)
    *handle = LoadLibrary(lib);
    if (*handle == NULL) {
        printf("Error: %d\n", GetLastError());
        return 1;
    }
#else
    *handle = dlopen(lib, flags);
    if (*handle == NULL) {
        printf("Error: %s\n", dlerror());
        return 1;
    }
#endif

    return 0;
}

static inline int dyn_sym(dyn_handle_t handle, char *sym, void (**fp)(void)) {
#if defined(WIN32) || defined(_WIN32)
    *fp = (void (*)(void))GetProcAddress(handle, sym);
    if (*fp == NULL) {
        printf("Error: %d\n", GetLastError());
        FreeLibrary(handle);
        return 1;
    }
#else
    union {
        void *ptr;
        void (*fp)(void);
    } cast;

    cast.ptr = dlsym(handle, sym);

    if (cast.ptr == NULL) {
        printf("Error: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    *fp = cast.fp;
#endif

    return 0;
}

static inline void dyn_close(dyn_handle_t handle) {
#if defined(WIN32) || defined(_WIN32)
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
}

static inline void dyn_symbol_info(void* symbol) {
#if !defined(WIN32) && !defined(_WIN32) && !defined(__APPLE__)
    Dl_info info;
    if (dladdr(symbol, &info)) {
        printf("Symbol: %s\n", info.dli_sname);
        printf("Address: %p\n", info.dli_saddr);
        printf("Library: %s\n", (info.dli_fname ? info.dli_fname : "unknown"));
    } else {
        printf("Error: Failed to get symbol info.\n");
    }
#endif
}

static inline void dyn_handle_info(dyn_handle_t handle) {
#if !defined(WIN32) && !defined(_WIN32) && !defined(__APPLE__)
    const struct link_map *link_map = 0;
    const int ret = dlinfo(handle, RTLD_DI_LINKMAP, &link_map);
    assert(link_map != 0);
    printf("Libraries:\n");
    while (link_map->l_prev) {
        printf("%s\n", link_map->l_name);
        link_map = link_map->l_prev;
    }
#endif
}


static inline dyn_handle_t dyn_current_process_handle(void) {
#if defined(WIN32) || defined(_WIN32)
    return GetModuleHandle(NULL);
#else
    return dlopen(NULL, RTLD_LAZY);
#endif
}

#endif
