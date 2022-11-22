#include "../lib/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_DIM 30000

void launch(struct Server* server) {
    char* welcome_message = "Hello! The server is all for you!";
    char buffer[BUFFER_DIM];
    
    while(1) {
        printf("\n[+] Server is now waiting for connections.\n");
        socklen_t address_len = sizeof(server->address);
        int new_socket = accept(server->socket, (struct sockaddr*) &server->address, &address_len);
        
        if (new_socket < 0) {
            perror("[-] Could not accept client connection: take a look at client's terminal.");
            exit(1); // exit(EXIT_FAILURE)
        }
        printf("[+] Client connection accepted.\n");
        // struct sockaddr_in client_address;
        // int new_client_socket = accept(server->socket, (struct sockaddr*) &client_address, &address_len);
        // printf("\n[+] Client %s, connection accepted.\n", inet_ntoa(client_address.sin_addr));

        read(new_socket, buffer, BUFFER_DIM);
        printf("    [+ +] Message received from client: %s", buffer);

        write(new_socket, welcome_message, strlen(welcome_message));
        // send(client_socket, welcome_message, strlen(welcome_message), 0); || send(client_socket, welcome_message, sizeof(welcome_message), 0);
        
        printf("    [+ +] Server's welcome message sent to client.\n");
        
        close(new_socket);
    }
}

int main() {
    struct Server server = create_server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 6969, 10, launch); // INADDR_LOOPBACK
    server.launch(&server);
    return 0;
}