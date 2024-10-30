#include <stdio.h>
#include <stdlib.h>

#include "server.h"

int main(int argc, char *argv[]) {
    /* char *file; */
    /* printf("Argument %d: %s\n", 1, argv[1]); */

    struct tftp_server s = {.port = 69, .file = "payload.txt"};
    int ret = start_server(&s);
    exit(ret);
}
