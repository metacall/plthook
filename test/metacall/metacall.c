#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <plthook.h>
#include "dynlib.h"

typedef char *(*string_function_ptr)(void);

string_function_ptr node_loader_fp = NULL;
string_function_ptr string_fp = NULL;
string_function_ptr string_fp2 = NULL;

int load_normal_executable() {
    plthook_t *plthook_node_loader;
    dyn_handle_t libnode2, node_loader;

    printf("METACALL load from normal executable\n");

    // Load NodeJS Loader dependency (libnode2)
    if (dyn_open(DYN_LIBRARY_PATH("./", "libnode2"), RTLD_GLOBAL | RTLD_LAZY, &libnode2) != 0) {
        return 1;
    }

    // Load NodeJS Loader
    if (dyn_open(DYN_LIBRARY_PATH("./", "node_loader"), RTLD_GLOBAL | RTLD_LAZY, &node_loader) != 0) {
        return 2;
    }

    // Load NodeJS Loader symbol
    if (dyn_sym(node_loader, "node_loader", (void (**)(void))&node_loader_fp) != 0) {
        return 3;
    }

    // In windows, we must relink node_loader dependencies to libnode2, it should work with
    // MacOs and Linux by default, but even that I prefer to be truly sure

    // Get all symbols of libnode (TODO: Here we should list all symbols of each dependency library
    // and store all of them in the hash map, this must be cross-platform)
    assert(dyn_sym(libnode2, "string_function", (void (**)(void))&string_fp) == 0);
    assert(strcmp("libnode2", string_fp()) == 0);
    assert(dyn_sym(libnode2, "string_function2", (void (**)(void))&string_fp2) == 0);
    assert(strcmp("libnode2", string_fp2()) == 0);

    // Patch the node_loader
    {
        if (plthook_open_by_handle(&plthook_node_loader, node_loader) != 0) {
            printf("plthook_open error: %s\n", plthook_error());
            return -1;
        }

        // Get all the symbols of node_loader
        unsigned int pos = 0;
        const char *name;
        void **addr;

        while (plthook_enum(plthook_node_loader, &pos, &name, &addr) == 0) {
            printf("NORMAL EXECUTABLE enum symbols node_loader (%d): %s -> %p\n", pos, name, addr);
            // Compare symbols from node_loader against the available symbols (of libnode, but this will
            // be done for each library dependency of node_loader)
            if (strcmp("string_function", name) == 0) {
                // Link the node_loader "string_function" to the function pointer string_function of libnode2
                if (plthook_replace(plthook_node_loader, "string_function", (void*)string_fp, (void**)NULL) != 0) {
                    printf("plthook_replace error: %s\n", plthook_error());
                    plthook_close(plthook_node_loader);
                    return -1;
                }
            }
            if (strcmp("string_function2", name) == 0) {
                // Link the node_loader "string_function2" to the function pointer string_function of libnode2
                if (plthook_replace(plthook_node_loader, "string_function2", (void*)string_fp2, (void**)NULL) != 0) {
                    printf("plthook_replace error: %s\n", plthook_error());
                    plthook_close(plthook_node_loader);
                    return -1;
                }
            }
            // Continue for all symbols...
            // We should have a hash map of all the symbols of each library dependency of node_loader
        }
    }

    // Execute the code
    char *str = node_loader_fp();
    printf("NORMAL EXECUTABLE executing string_function from: %s\n", str);
    assert(strcmp(str, "libnode2") == 0);

    // Destroy everything
    plthook_close(plthook_node_loader);
    dyn_close(node_loader);
    dyn_close(libnode2);

    return 0;
}

int load_node_dynamic(void) {
    plthook_t *plthook_node_loader;
    dyn_handle_t libnode, node_loader;

    printf("METACALL load from node compiled dynamically to libnode\n");

    // TODO: Here it is not implemented, but we should get all the dependencies of the current process
    // List all the loaded libraries and try to find each of the dependencies list of the node_loader
    // on that list of loaded libraries.
    // So for example if node.exe is linked against libnode, and we have libnode in the configuration
    // dependency list, then we get the node.exe libnode reference and link node_loader to it.

    // Load node.exe dependency (libnode)
    if (dyn_open(DYN_LIBRARY_PATH("./", "libnode"), RTLD_GLOBAL | RTLD_LAZY, &libnode) != 0) {
        return 1;
    }

    // Load NodeJS Loader
    if (dyn_open(DYN_LIBRARY_PATH("./", "node_loader"), RTLD_GLOBAL | RTLD_LAZY, &node_loader) != 0) {
        return 2;
    }

    // Load NodeJS Loader symbol
    if (dyn_sym(node_loader, "node_loader", (void (**)(void))&node_loader_fp) != 0) {
        return 3;
    }

    // In windows, we must relink node_loader dependencies to libnode, it should work with
    // MacOs and Linux by default, but even that I prefer to be truly sure

    // Get all symbols of libnode (TODO: Here we should list all symbols of each dependency library
    // and store all of them in the hash map, this must be cross-platform)
    assert(dyn_sym(libnode, "string_function", (void (**)(void))&string_fp) == 0);
    assert(strcmp("node-dynamic", string_fp()) == 0);

    // We simulate another version of libnode here, it won't have "string_function2" because they have diffent APIs
    // assert(dyn_sym(libnode, "string_function2", (void (**)(void))&string_fp2) == 0);

    // Patch the node_loader
    {
        if (plthook_open_by_handle(&plthook_node_loader, node_loader) != 0) {
            printf("plthook_open error: %s\n", plthook_error());
            return -1;
        }

        // Get all the symbols of node_loader
        unsigned int pos = 0;
        const char *name;
        void **addr;

        while (plthook_enum(plthook_node_loader, &pos, &name, &addr) == 0) {
            printf("NODE DYNAMIC enum symbols node_loader (%d): %s -> %p\n", pos, name, addr);
            // Compare symbols from node_loader against the available symbols (of libnode, but this will
            // be done for each library dependency of node_loader)
            if (strcmp("string_function", name) == 0) {
                // Link the node_loader "string_function" to the function pointer string_function of libnode
                if (plthook_replace(plthook_node_loader, "string_function", (void*)string_fp, (void**)NULL) != 0) {
                    printf("plthook_replace error: %s\n", plthook_error());
                    plthook_close(plthook_node_loader);
                    return -1;
                }
            }
            // We simulate another version of libnode here, it won't have "string_function2" because they have diffent APIs
            // if (strcmp("string_function2", name) == 0) {
            //     // Link the node_loader "string_function2" to the function pointer string_function of libnode
            //     if (plthook_replace(plthook_node_loader, "string_function2", (void*)string_fp2, (void**)NULL) != 0) {
            //         printf("plthook_replace error: %s\n", plthook_error());
            //         plthook_close(plthook_node_loader);
            //         return -1;
            //     }
            // }
            // Continue for all symbols...
            // We should have a hash map of all the symbols of each library dependency of node_loader
        }
    }

    // Execute the code
    char *str = node_loader_fp();
    printf("NODE DYNAMIC executing string_function from: %s\n", str);
    assert(strcmp(str, "node-dynamic") == 0);

    // Destroy everything
    plthook_close(plthook_node_loader);
    dyn_close(node_loader);
    dyn_close(libnode);

    return 0;
}

int load_node_static(char *(*string_function_static)(void)) {
    plthook_t *plthook_node_loader;
    dyn_handle_t current_process, node_loader;

    // TODO: In theory we should test linking against libmetacall.a, but I think there won't be
    // difference if we just define the function in the same executable)
    printf("METACALL load from node compiled statically (the functions are in the executable)\n");

    dyn_symbol_info(string_function_static);

    // TODO: Here it is not implemented, but we should get all the dependencies of the current process
    // List all the loaded libraries and try to find each of the dependencies list of the node_loader
    // on that list of loaded libraries.
    // So for example if node.exe is linked statically against libnode, and we do not find any,
    // we should link against the current process

    // In fact maybe it would be interesting to define a priority, maybe we can try to link first against
    // the current process and if we do not find the symbol, then try to link against the hash map generated
    // from the match of listing the executable dependencies and the dependency list in the configuration

    // Get handle of current process
    current_process = dyn_current_process_handle();

    // Load NodeJS Loader
    if (dyn_open(DYN_LIBRARY_PATH("./", "node_loader"), RTLD_GLOBAL | RTLD_LAZY, &node_loader) != 0) {
        return 2;
    }

    // Load NodeJS Loader symbol
    if (dyn_sym(node_loader, "node_loader", (void (**)(void))&node_loader_fp) != 0) {
        return 3;
    }

    // In windows, we must relink node_loader dependencies to current process, it should work with
    // MacOs and Linux by default, but even that I prefer to be truly sure

    // Get all symbols of current_process (TODO: Here we should list all symbols of each dependency library
    // and store all of them in the hash map, this must be cross-platform)
    // TODO: Review this in MacOS https://github.com/jslegendre/libSymRez
    assert(dyn_sym(current_process, "string_function", (void (**)(void))&string_fp) == 0);
    printf("string_function: %p == %p\n", string_function_static, string_fp);
    assert(string_function_static == string_fp);
    assert(strcmp("node-static", string_fp()) == 0);

    // We simulate another version of libnode here, it won't have "string_function2" because they have diffent APIs
    // assert(dyn_sym(current_process, "string_function2", (void (**)(void))&string_fp2) == 0);

    // Patch the node_loader
    {
        if (plthook_open_by_handle(&plthook_node_loader, node_loader) != 0) {
            printf("plthook_open error: %s\n", plthook_error());
            return -1;
        }

        // Get all the symbols of node_loader
        unsigned int pos = 0;
        const char *name;
        void **addr;

        while (plthook_enum(plthook_node_loader, &pos, &name, &addr) == 0) {
            printf("NODE STATIC enum symbols node_loader (%d): %s -> %p\n", pos, name, addr);
            // Compare symbols from node_loader against the available symbols (of current_process, but this will
            // be done for each library dependency of node_loader)
            if (strcmp("string_function", name) == 0) {
                // Link the node_loader "string_function" to the function pointer string_function of current_process
                if (plthook_replace(plthook_node_loader, "string_function", (void*)string_fp, (void**)NULL) != 0) {
                    printf("plthook_replace error: %s\n", plthook_error());
                    plthook_close(plthook_node_loader);
                    return -1;
                }
            }
            // We simulate another version of current_process here, it won't have "string_function2" because they have diffent APIs
            // if (strcmp("string_function2", name) == 0) {
            //     // Link the node_loader "string_function2" to the function pointer string_function of current_process
            //     if (plthook_replace(plthook_node_loader, "string_function2", (void*)string_fp2, (void**)NULL) != 0) {
            //         printf("plthook_replace error: %s\n", plthook_error());
            //         plthook_close(plthook_node_loader);
            //         return -1;
            //     }
            // }
            // Continue for all symbols...
            // We should have a hash map of all the symbols of each library dependency of node_loader
        }
    }

    // Execute the code
    char *str = node_loader_fp();
    printf("NODE STATIC executing string_function from: %s\n", str);
    assert(strcmp(str, "node-static") == 0);

    // Destroy everything
    plthook_close(plthook_node_loader);
    dyn_close(node_loader);

    return 0;
}
