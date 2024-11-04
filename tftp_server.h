struct tftp_server {
    int port;
    int retries;
    char *file;
};


int start_server(struct tftp_server *s);

/**
 * Transfers the specified source file to the addressed client using the given
 * client in BINARY mode transfer.
 *
 * @param  src_file  source file to be transferred;
 * @param  socket    socket to be used to transfer the file to the recipient
 *                   client;
 * @param  cli_addr  address of the client requesting the file transfer.
 */
/* void binary_mode_transfer(FILE *src_file, int socket, struct sockaddr cli_addr); */
