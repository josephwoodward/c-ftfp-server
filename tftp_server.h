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

typedef union {

    uint16_t opcode;

    struct {
	uint16_t opcode; /* RRQ or WRQ */
	uint8_t filename_and_mode[514];
    } request;

    struct {
	uint16_t opcode; /* DATA */
	uint16_t block_number;
	uint8_t data[512];
    } data;

    struct {
	uint16_t opcode; /* ACK */
	uint16_t block_number;
    } ack;

    struct {
	uint16_t opcode; /* ERROR */
	uint16_t error_code;
	uint8_t error_string[512];
    } error;

} tftp_message;


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

int parse_request(int socket_desc, tftp_message *m,struct sockaddr_in *client_addr);
