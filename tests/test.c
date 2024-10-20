#include <stdio.h>
#include <stdlib.h>

#include "../server.c"

#if _WIN32
#define C_RED(s) s
#define C_GREEN(s) s
#define C_YELLOW(s) s
#define C_MAGENTA(s) s
#else
#define C_RED(s) "\033[31;1m" s "\033[0m"
#define C_GREEN(s) "\033[32;1m" s "\033[0m"
#define C_YELLOW(s) "\033[33;1m" s "\033[0m"
#define C_MAGENTA(s) "\033[35;1m" s "\033[0m"
#endif

int main(void) {
    int fails = 0;
    struct tftp_server s = {.port = 8082};

    {
	// start_server tests
	if (start_server(&s) > 0) {
	    printf(C_RED("FAILED: ") "start_server expected to be 0 but was 1\n");
	} else {
	    printf(C_GREEN("PASS: ") "start_server started successfully\n");
	}
    }

    return fails ? EXIT_FAILURE : EXIT_SUCCESS;
}
