#include <stdlib.h>

#include "../server.c"

int main(void) {
    int fails = 0;
    struct tftp_server s = {.port = 8082};

    {
	fails += start_server(&s);
    }

    return fails ? EXIT_FAILURE : EXIT_SUCCESS;
}
