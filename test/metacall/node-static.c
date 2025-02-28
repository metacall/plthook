#include <stdio.h>
#include "metacall.h"

// #if defined(WIN32) || defined(_WIN32)
// 	#define EXPORT __declspec(dllexport)
// #else
// 	#define EXPORT __attribute__((visibility("default")))
// #endif

// EXPORT char *string_function(void) {
//     return "node-static";
// }

#include "libnode.h"

int main() {
    printf("%s\n", string_function());
    return load_node_static(&string_function);
}
