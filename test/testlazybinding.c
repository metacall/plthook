#include <stddef.h>
#include "libtest.h"

/*
https://github.com/metacall/plthook/issues/4
https://github.com/kubo/plthook/pull/55#issuecomment-2863552101
*/
double strtod_lazy_binding(void)
{
    double num = strtod_cdecl("3.7", NULL);
    return num;
}
