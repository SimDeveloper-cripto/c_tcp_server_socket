#include "../include/server.h"
#include <stdio.h>
#include <stdlib.h>

struct Server create_server(int address_family, int service, int protocol, u_long interface, int port, 
    int backlog, void (*launch)(struct Server* server)) {
    int opt = 1;
    struct Server server;

    server.address_family = address_family;
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog;

    server.address.sin_family = address_family;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    // Lets create the socket; we return a socket descriptor.
    server.socket = socket(server.address_family, server.service, server.protocol);
    if (server.socket < 0) {
        perror("[-] Socket failed.\n");
        exit(1); // Exiting with code 1 implies a failure.
    }
    puts("[+] Server socket created.");

    if (setsockopt(server.socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("[-] setsockopt failed.\n");
        exit(1);
    }

    if (bind(server.socket, (struct sockaddr*) &server.address, sizeof(server.address)) < 0) {
        perror("[-] Socket bind failed.");
        exit(1);
    }
    puts("[+] Socket bin successful.");

    if (listen(server.socket, server.backlog) < 0) {
        perror("[-] Failed to start listening.\n");
        exit(1);
    }

    server.launch = launch;
    printf("[+] Sever is up at localhost: %d\n", server.port);

    return server;
}