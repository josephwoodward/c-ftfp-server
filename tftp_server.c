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

    int socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc < 0) {
	printf("Error while creating socket\n");
	return -1;
    }

    printf("Socket created successfully\n");

    // configure socket to allow it to be reused, avoiding 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
	printf("SO_REUSEPORT failed\n");
	return -1;
    }

    // Set port and IP:

    /* server_sock.sin_family = AF_INET; */
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
		return -1;
	    }

	    transfer_binary_mode(file, socket_desc, &client_addr);
	    return EXIT_SUCCESS;
	}
    }

    return EXIT_SUCCESS;
}

int parse_message(int socket_desc, tftp_message *m, struct sockaddr_in *client_addr) {
    socklen_t clientAddrLen = sizeof(&client_addr);
    int bytes_read = recvfrom(socket_desc, m, sizeof(*m), 0, (struct sockaddr *)client_addr, &clientAddrLen);
    if (bytes_read < 0) {
	printf("Error whilst listening for tftp message: %d", errno);
	return -1;
    }

    m->opcode = ntohs(m->opcode);

    switch (m->opcode) {
    case OPCODE_RRQ:
	/* 2 bytes     string    1 byte     string   1 byte */
	/* ------------------------------------------------ */
	/* | Opcode |  Filename  |   0  |    Mode    |   0  | */
	/* ------------------------------------------------ */

	// parse mode (positioned after filename, which could be variable length) by using
	// pointer arithmatic to move pointer beyond "filename\n" so we can read mode string.
	strcpy(m->request.mode, m->request.filename_and_mode + 1 + strlen(m->request.filename_and_mode));
	break;
    case OPCODE_ACK:
	/* 2 bytes     2 bytes */
	/* --------------------- */
	/* | Opcode |   Block #  | */
	/* --------------------- */
	// statements
	break;
    }

    if (DEBUG) {
	printf("root opcode received: %d\n", m->opcode);
	printf("request filename: %s\n", m->request.filename_and_mode);
	printf("mode: %s\n", m->request.mode);
    }

    return 0;
}

void transfer_binary_mode(FILE *src_file, int socket_desc, struct sockaddr_in *client_addr) {
    // prepare response message
    // Data acts as the data packet that will transfer the files payload
    // 2 bytes     2 bytes      n bytes
    // ----------------------------------
    // | Opcode |   Block #  |   Data     |
    // ----------------------------------

    uint16_t block = htons(1);

    uint16_t block_num, stop_sending = 0;
    ssize_t contents_len;
    uint8_t file_content[512];

    printf("tansferring file...\n");

    /* while (stop_sending == 0) { */
    contents_len = fread(file_content, 1, sizeof(file_content), src_file);
    block_num++;

    tftp_message m;
    m.opcode = htons(OPCODE_DATA);
    m.data.block_number = block_num;
    memcpy(m.data.data, file_content, contents_len);

    // can we send entire file in single data frame?
    if (contents_len < 512) {
	stop_sending = 1;
    }

    ssize_t len = sizeof(m.opcode) + sizeof(m.data.block_number) + contents_len;
    int bytes_sent = sendto(socket_desc, &m, len, 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
    if (bytes_sent < 0) {
	printf("failed to send to client: %s\n", strerror(errno));
	return;
    }

    printf("%d sent successfully\n", bytes_sent);

    return;
}
