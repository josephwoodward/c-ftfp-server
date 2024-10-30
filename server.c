#include "server.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define DATAGRAM_SIZE 516

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

    printf("Listening for incoming messages...\n\n");

    char buf[DATAGRAM_SIZE];
    /* int from_len = sizeof(serv_addr); */
    socklen_t clientAddrLen = sizeof(serv_addr);
    int bytes;
    while (1) {
	/* fromlen = sizeof(addr); */
	bytes = recvfrom(socket_desc, buf, sizeof(buf), 0, (struct sockaddr *)&serv_addr, &clientAddrLen);
	printf("recv()'d %d bytes of data in buf\n", bytes);
	/* printf("from IP address %s\n", inet_ntoa(addr.sin_addr)); */
    }

    return EXIT_SUCCESS;
}
