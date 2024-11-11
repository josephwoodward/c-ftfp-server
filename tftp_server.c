#include "tftp_server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define DATAGRAM_SIZE 516

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
    int bytes_read;

    memset(buf, 0, sizeof buf);

    // this will either be "netascii", "octet" or "mail"
    char mode[8];

    while (1) {
	// read contents of dataframe
	bytes_read =
	    recvfrom(socket_desc, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &clientAddrLen);
	if (bytes_read < 0) {
	    printf("Error whilst listening for tftp packets: %d", errno);
	    return -1;
	}

	// parse opcode
	uint16_t opcode = ntohs(*(uint16_t *)&buf);
	printf("opcode received: %d\n", opcode);

	// parse filename
	int p = 2;
	char *req_filename = calloc(strlen(buf + p), sizeof(char));
	if (req_filename == NULL) {
	    printf("Error whilst parsing filename: %d\n", errno);
	    return -1;
	}
	strcpy(req_filename, buf + p);

	if (DEBUG) {
	    printf("filename received: %s, length is %lu\n", req_filename, strlen(req_filename));
	}

	// parse mode
	p++; // account for zero byte
	memset(mode, 0, sizeof(mode));
	strcpy(mode, buf + p + strlen(req_filename));

	if (DEBUG) {
	    printf("mode is: %s\n", mode);
	}

	if (strcmp(mode, "octet") == 0) {
	    transfer_binary_mode(NULL, socket_desc, &client_addr);
	}

	free(req_filename);
    }

    return EXIT_SUCCESS;
}

// Data acts as the data packet that will transfer the files payload
// 2 bytes     2 bytes      n bytes
// ----------------------------------
// | Opcode |   Block #  |   Data     |
// ----------------------------------
void transfer_binary_mode(FILE *src_file, int socket_desc, struct sockaddr_in *client_addr) {

    // clear buffer before writing to it
    memset(buf, 0, sizeof buf);
    /* memset(buf, 0, BUFSIZE); //can this work instead? */

    uint16_t opcode = ntohs(OPCODE_ACK);
    memcpy(buf, &opcode, 2);

    uint16_t block = htons(1);
    memcpy(buf, &block, 2);

    fwrite(buf, 1, sizeof(buf), stdout);

    const char *msg = "hello world\0";
    memcpy(buf, "Hello world\0", strlen(msg));

    /* printf("lengfth of buuffer: %s\n", strlen(buf + 2)); */

    printf("transferring binary 1\n");

    int result = sendto(socket_desc, msg, strlen(msg), 0, (struct sockaddr *)&client_addr, sizeof(*client_addr));
    printf("result is: %d\n", result);
}
