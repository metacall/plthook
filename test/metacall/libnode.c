#include "libnode.h"

char *string_function(void) {
#ifdef NODE_STATIC
    return "node-static";
#else
    return "node-dynamic";
#endif
}
