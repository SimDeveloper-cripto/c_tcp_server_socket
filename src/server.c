#include "../lib/server.h"
#include <stdio.h>
#include <stdlib.h>

struct Server server_constructor(int domain, int service, int protocol, u_long interface, int port, 
    int backlog, void (*launch)(struct Server* server))
{
    struct Server server;

    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog; // coda di client

    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.socket = socket(domain, service, protocol);
    if (server.socket == 0) {
        perror("[-] Socket failed.\n");
        exit(1); // exit with code 1 implies a failure
    }
    puts("[+] Server socket created.");

    if (bind(server.socket, (struct sockaddr*)&server.address, sizeof(server.address)) < 0) {
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