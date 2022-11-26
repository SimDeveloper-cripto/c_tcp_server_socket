#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// ...

struct Server {
    int address_family; // To specify the address family we are using. In our case an Internet Protocol address.
    int service;        // To specify a connection-based protocol. In our case Transfer Control Protocol.
    int protocol;
    u_long interface;
    int port;
    int backlog;        // To specify the amount of clients our server can handle.
    
    int socket;         // It specifies the socket descriptor.

    struct sockaddr_in address;

    void (*launch) (struct Server* server);
};

// [SERVER CONSTRUCTOR]
struct Server create_server(int address_family, int service, int protocol, u_long interface, int port,
    int backlog, void (*launch)(struct Server* server));

#endif /* SERVER_H */