#include "../lib/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <mysql.h>

// HERE WE LIST MYSQL CONNECTION VARIABLES
char* server = "localhost";
char* user = " ";          /* Remeber to change this value. */
char* password = " ";      /* Remeber to change this value. */
char* database = " ";      /* Remeber to change this value. */

#define BUFFER_DIM 30000

// new_socket is the client_socket_descriptor.
void* connection_handler(void* socket_desc) {
    int new_socket = (*(int*) socket_desc);
    char* welcome_message = "Hello! The server is all for you!";
    char buffer[BUFFER_DIM];
    read(new_socket, buffer, BUFFER_DIM);
    printf("    [+ +] Message received from client: %s", buffer);
    write(new_socket, welcome_message, strlen(welcome_message));
    // send(client_socket, welcome_message, strlen(welcome_message), 0); || send(client_socket, welcome_message, sizeof(welcome_message), 0);
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

int main() {
    // struct Server server = create_server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 6969, 10, launch); // INADDR_LOOPBACK
    // server.launch(&server);

    MYSQL* connection;
    MYSQL_RES* result;
    MYSQL_ROW row;
    int step = 0;

    /* Connect to MySQL dbms */
    connection = mysql_init(NULL);

    if (!mysql_real_connect(connection, server, user,
        password, database, 0, NULL, 0)) {
            fprintf(stderr, "%s\n", mysql_error(connection));
            exit(1);
    }

	/* How to send SQL query to the MySQL dmbs */
	if (mysql_query(connection, "select nome from utenti")) {
		fprintf(stderr, "%s\n", mysql_error(connection));
		exit(1);
	}
   
    /* Get the result */
	result = mysql_use_result(connection);
	
	/* Output the result */
	printf("Users found inside the table 'utenti'\n");
	while ((row = mysql_fetch_row(result)) != NULL) {
		++step;
        printf("%d. ['%s']\n", step, row[0]);
    }
   
	/* Close connection */
	mysql_free_result(result);
	mysql_close(connection);
    return 0;
}