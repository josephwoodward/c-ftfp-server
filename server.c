#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int start_server(struct tftp_server *s) {
    if (s->file == NULL || strlen(s->file) < 1) {
	perror("file to transfer not specified");
	return EXIT_FAILURE;
    }

    if (s->port == 0) {
	perror("port not specified");
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
