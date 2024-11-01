#include "server.h"

#include <arpa/inet.h>
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
	printf("Couldn't bind to the port %d\n", s->port);
	return 1;
    }

    printf("Listening for incoming tftp messages on port %d...\n\n", s->port);

    char buf[DATAGRAM_SIZE];
    memset(buf, 0, sizeof buf);

    socklen_t clientAddrLen = sizeof(serv_addr);
    int bytes_read;

    while (1) {
	// read contents of dataframe
	bytes_read = recvfrom(socket_desc, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&serv_addr, &clientAddrLen);
	if (bytes_read < 0) {
	    printf("Error whilst listening for tftp packets");
	    return -1;
	}

	// parse opcode
	uint16_t opcode = ntohs(*(uint16_t *)&buf);
	printf("opcode received: %d\n", opcode);

	// parse filename
	char *filename = calloc(strlen(buf + 2), sizeof(char));
	if (filename == NULL) {
	    printf("Error whilst parsing filename\n");
	    return -1;
	}
	strcpy(filename, buf + 2);
	printf("filename received: %s\n", filename);

	free(filename);
    }

    return EXIT_SUCCESS;
}
