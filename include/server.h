#ifndef SERVER_H
#define SERVER_H

#include <mysql.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <json-c/json.h>

// ...

typedef struct {
    int address_family; // To specify the address family we are using. In our case an Internet Protocol address.
    int service;        // To specify a connection-based protocol. In our case Transfer Control Protocol.
    int protocol;
    u_long interface;
    int port;
    int backlog;        // To specify the amount of clients our server can handle.
    
    int socket;         // It specifies the socket descriptor.

    struct sockaddr_in address;
} server_t;

// [SERVER CONSTRUCTOR]
server_t create_server(int address_family, int service, int protocol, u_long interface, int port, int backlog);

// SERVER'S MAIN FUNCTION
int launch(server_t* server);

// SERVER RELATED FUNCTIONS
bool exists(MYSQL* connection, char query[], pthread_mutex_t lock);
void send_random_code(int new_socket);
void manage_login(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock);
void manage_register(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock);
void manage_forgot_password(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock);
void manage_alter_password(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock);
void manage_get_ticket(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock);
void manage_check_ticket_acquired(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock);

#endif /* SERVER_H */