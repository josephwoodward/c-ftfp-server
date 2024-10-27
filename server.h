struct tftp_server {
    int port;
    char *file;
};

int start_server(struct tftp_server *s);
