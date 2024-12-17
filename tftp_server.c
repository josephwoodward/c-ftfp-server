#include "tftp_server.h"

#include <_string.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATAGRAM_SIZE 516
#define BLOCKSIZE = DATAGRAM_SIZE - 4;

#define OPCODE_RRQ 1
#define OPCODE_WRQ 2
#define OPCODE_DATA 3
#define OPCODE_ACK 4
#define OPCODE_ERR 5

#undef DEBUG
#define DEBUG 1

FILE *src_file;

// buffer used for both reading and writing data to packets
char buf[DATAGRAM_SIZE];

int start_server(struct tftp_server *s) {
    if (s->file == NULL || strlen(s->file) < 1) {
	perror("file to transfer not specified");
	return EXIT_FAILURE;
    }

    if (s->port == 0) {
	perror("port not specified");
	return EXIT_FAILURE;
    }

    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_desc < 0) {
	printf("Error while creating socket\n");
	return -1;
    }

    printf("Socket created successfully\n");

    // configure socket to allow it to be reused, avoiding 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
	printf("SO_REUSEPORT failed\n");
	return 1;
    }

    // Set port and IP:
    struct sockaddr_in serv_addr = {
	.sin_family = AF_INET,		 // we are using Ipv4 protocol
	.sin_port = htons(s->port),	 // port 2053
	.sin_addr = {htonl(INADDR_ANY)}, // 0.0.0.0
    };

    // Bind to the set port and IP so we don't receive everything
    if (bind(socket_desc, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
	printf("Couldn't bind to the port %d: %d\n", s->port, errno);
	return -1;
    }

    printf("Listening for incoming tftp messages on port %d...\n\n", s->port);

    struct sockaddr_in client_addr;
    socklen_t clientAddrLen = sizeof(client_addr);

    // ensure buffer is set
    memset(buf, 0, sizeof buf);

    // mode is either "netascii", "octet" or "mail"
    char mode[8];
    tftp_message m;

    while (1) {
	// read contents of dataframe
	if (parse_message(socket_desc, &m, &client_addr) != 0) {
	    printf("failed to parse tftp message\n", errno);
	    return -1;
	}

	if (strcmp(m.request.mode, "octet") == 0) {
	    FILE *file;
	    file = fopen(m.request.filename_and_mode, "r");
	    if (file == NULL) {
		printf("Unable to open the file.\n");
	    }

	    transfer_binary_mode(file, socket_desc, &client_addr);
	}
    }

    return EXIT_SUCCESS;
}

/* 2 bytes     string    1 byte     string   1 byte */
/* ------------------------------------------------ */
/* | Opcode |  Filename  |   0  |    Mode    |   0  | */
/* ------------------------------------------------ */
int parse_message(int socket_desc, tftp_message *m, struct sockaddr_in *client_addr) {
    socklen_t clientAddrLen = sizeof(&client_addr);
    int bytes_read = recvfrom(socket_desc, m, sizeof(*m), 0, (struct sockaddr *)&client_addr, &clientAddrLen);
    if (bytes_read < 0) {
	printf("Error whilst listening for tftp message: %d", errno);
	return -1;
    }

    m->opcode = ntohs(m->opcode);
    if (m->opcode == OPCODE_RRQ) {
	strcpy(m->request.mode, m->request.filename_and_mode + 1 + strlen(m->request.filename_and_mode));
    }

    if (DEBUG) {
	printf("root opcode received: %d\n", m->opcode);
	printf("request filename: %s\n", m->request.filename_and_mode);
	printf("mode: %s\n", m->request.mode);
    }

    return 0;
}

void transfer_binary_mode(FILE *src_file, int socket_desc, struct sockaddr_in *client_addr) {
    uint8_t data[512];

    uint16_t block_number = 0;

    /* int countdown; */
    /* int to_close = 0; */
    // Store the content of the file
    /* char myString[100] = {0}; */
    /* fgets(myString, 100, src_file); */

    // prepar buffer before writing it to the wire
    memset(buf, 0, sizeof buf);

    // prepare response message
    // Data acts as the data packet that will transfer the files payload
    // 2 bytes     2 bytes      n bytes
    // ----------------------------------
    // | Opcode |   Block #  |   Data     |
    // ----------------------------------
    uint16_t opcode = ntohs(OPCODE_ACK);
    memcpy(buf, &opcode, 2);

    uint16_t block = htons(1);
    memcpy(buf, &block, 2);

    const char *msg = "hello world\0";
    memcpy(buf, "Hello world\0", strlen(msg));

    // TODO: Loop and send contents of file

    uint16_t block_num = 0;
    uint16_t stop_sending = 1;

    ssize_t slen, c;

    while (!stop_sending) {
	slen = fread(data, 1, sizeof(data), src_file);
	block_num++;

	// size is within max payload size
	if (slen < 512) {
	    stop_sending = 1;
	}

	int result = sendto(socket_desc, data, strlen(data), 0, (struct sockaddr *)&client_addr, sizeof(*client_addr));
	printf("result is: %d\n", result);
    }
}
