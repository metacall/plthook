#ifndef DYNLYB
#define DYNLYB 1

#include <stdio.h>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
typedef HMODULE dyn_handle_t;
#define DYN_EXT "dll"
#define RTLD_LAZY 0 // Use this as workaround
#define RTLD_GLOBAL 0
#define DYN_PRE
#else
#include <dlfcn.h>
typedef void* dyn_handle_t;
#ifdef __APPLE__
#define DYN_EXT ".dylib"
#define DYN_PRE "_" // Use this as workaround
#else
#define DYN_EXT ".so"
#define DYN_PRE
#endif
#endif

static inline int dyn_open(char *lib, int flags, dyn_handle_t *handle) {
#if defined(WIN32) || defined(_WIN32)
    *handle = LoadLibrary(lib);
    if (*handle == NULL) {
        printf("Error: %s\n", GetLastError());
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
        printf("Error: %s\n", GetLastError());
        FreeLibrary(handle);
        return 1;
    }
#else
    *fp = (void (*)(void))dlsym(handle, sym);
    if (*fp == NULL) {
        printf("Error: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }
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

static inline dyn_handle_t dyn_current_process_handle(void) {
#if defined(WIN32) || defined(_WIN32)
    return GetModuleHandle(NULL);
#else
    return dlopen(NULL, RTLD_LAZY);
#endif
}

#endif
