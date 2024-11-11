#include <stdio.h>
#include <arpa/inet.h>

/**
 * Set flag to 1 to enable debugging log messages.
 */
#define DEBUG 0

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
void transfer_binary_mode(FILE *src_file, int socket, struct sockaddr_in *cli_addr);
