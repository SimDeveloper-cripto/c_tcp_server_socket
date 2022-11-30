#include "../lib/server.h"
#include "../lib/main.h"
#include "../lib/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_DIM 30000

// SERVER RELATED FUNCTIONS PROTOTYPES
void* connection_handler(void* socket_desc);
void launch(struct Server* server);

// MYSQL RELATED FUNCTIONS PROTOTYPES
MYSQL* init_mysql_connection(MYSQL* connection, char* password);
MYSQL_RES* make_query_get_result(MYSQL* connection, char* query);
void make_query_print_result(MYSQL* connection, char* query);

int main() {
    int step = 0;
    MYSQL_ROW row;
    char* query = "select nome from utenti";
    
    // struct Server server = create_server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 6969, 10, launch); // INADDR_LOOPBACK
    // server.launch(&server);
    MYSQL* connection = init_mysql_connection(connection, util_read_password_from_file());

    printf("Users found inside the table 'utenti'\n");
    make_query_print_result(connection, query);
    
	// DON'T FORGET TO CLOSE MYSQL CONNECTION BEFORE ENDING THE PROGRAM
	mysql_close(connection);
    return 0;
}

// SERVER RELATED FUNCTIONS
void* connection_handler(void* socket_desc) {
    int new_socket = (*(int*) socket_desc);
    char* welcome_message = "Hello! The server is all for you!";
    char buffer[BUFFER_DIM];

    read(new_socket, buffer, BUFFER_DIM);
    printf("    [+ +] Message received from client: %s", buffer);

    write(new_socket, welcome_message, strlen(welcome_message));
    printf("    [+ +] Server's welcome message sent to client.\n");

    close(new_socket);
}

void launch(struct Server* server) {
    printf("[+] Server is now waiting for connections.\n");
    while(1) {
        socklen_t address_len = sizeof(server->address);
        pthread_t thread_id;

        int new_socket = accept(server->socket, (struct sockaddr*) &server->address, &address_len);
        
        if (new_socket < 0) {
            perror("[-] Could not accept client connection: take a look at client's terminal.");
            exit(1); // exit(EXIT_FAILURE)
        }
        printf("\n[+] Client connection accepted.\n");

        if (pthread_create(&thread_id, NULL, connection_handler, (void*) &new_socket) < 0) {
            perror("[-] Could not create thread.");
            exit(1);
        }
        printf("    [+ +] Thread created for client requests.\n");
        // struct sockaddr_in client_address;
        // int new_client_socket = accept(server->socket, (struct sockaddr*) &client_address, &address_len);
        // printf("\n[+] Client %s, connection accepted.\n", inet_ntoa(client_address.sin_addr));
    }
}

// MYSQL RELATED FUNCTIONS
MYSQL* init_mysql_connection(MYSQL* connection, char* password) {
    /* Connect to MySQL dbms */
    connection = mysql_init(NULL);
    if (!mysql_real_connect(connection, server, user, password, database, 0, NULL, 0)) {
            fprintf(stderr, "%s\n", mysql_error(connection));
            exit(1);
    }
    return connection;
}

MYSQL_RES* make_query_get_result(MYSQL* connection, char* query) {
    MYSQL_ROW row;
    int step = 0;
	if (mysql_query(connection, query)) {
		fprintf(stderr, "%s\n", mysql_error(connection));
		exit(1);
	}
	MYSQL_RES* result = mysql_use_result(connection);
	while ((row = mysql_fetch_row(result)) != NULL) {
		++step;
        printf("%d. ['%s']\n", step, row[0]);
    }
    return result;
}

void make_query_print_result(MYSQL* connection, char* query) {
    MYSQL_ROW row;
    int step = 0;
	if (mysql_query(connection, query)) {
		fprintf(stderr, "%s\n", mysql_error(connection));
		exit(1);
	}
    
	MYSQL_RES* result = mysql_use_result(connection);
	while ((row = mysql_fetch_row(result)) != NULL) {
		++step;
        printf("%d. ['%s']\n", step, row[0]);
    }
	mysql_free_result(result);
}