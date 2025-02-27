#include <stdio.h>
#include <assert.h>

#include "libnode2.h"

char *node_loader() {
    char *str = string_function();
    printf("INSIDE NODE LOADER: %s\n", str);
    return str;
}
