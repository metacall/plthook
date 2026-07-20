#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../plthook.h"

static pid_t my_fork(void)
{
    printf("my_fork hook called\n");
    return -1;
}

int main(void)
{
    plthook_t *plthook;
    unsigned int pos = 0;
    const char *name;
    void **addr;
    int rv;
    pid_t pid;

    if (plthook_open(&plthook, NULL) != 0) {
        printf("%s\n", plthook_error());
        return 1;
    }

    while (plthook_enum(plthook, &pos, &name, &addr) == 0) {
        printf("%s\n", name);
    }

    rv = plthook_replace(plthook, "fork", (void *)my_fork, NULL);
    if (rv == 0) {
        printf("replaced\n");
    } else {
        printf("not replaced: %s\n", plthook_error());
    }

    plthook_close(plthook);

    pid = fork();
    if (pid == 0) {
        _exit(0);
    }
    wait(NULL);

    return 0;
}
