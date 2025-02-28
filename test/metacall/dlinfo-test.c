#include <stdio.h>

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>
#include <link.h>

int main() {
    // Open a shared library
    const char *lib_name = "./liblibnode.so";  // For example, the math library
    void *handle = dlopen(lib_name, RTLD_NOW);

    if (!handle) {
        fprintf(stderr, "Error opening library %s: %s\n", lib_name, dlerror());
        return 1;
    }

    // Retrieve the dynamic link map of the shared object using dlinfo
    struct link_map *map;
    if (dlinfo(handle, RTLD_DI_LINKMAP, &map) == 0) {
        printf("Link map of %s:\n", lib_name);
        while (map) {
            printf("  Name: %s, Address: %ld\n", map->l_name, map->l_addr);
            map = map->l_next;
        }
    } else {
        fprintf(stderr, "dlinfo failed: %s\n", dlerror());
    }

    // Use dlsym to find a symbol in the library (e.g., "sin" in libm.so)
    void *symbol = dlsym(handle, "sin");
    if (symbol) {
        printf("Found symbol 'sin' at address: %p\n", symbol);
    } else {
        printf("Symbol 'sin' not found: %s\n", dlerror());
    }

    // Close the library handle
    dlclose(handle);
    return 0;
}
