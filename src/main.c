#include "../include/server.h"
#include "../include/list.h"
#include "../include/main.h"

#define UTILS_H_IMPLEMENTATION
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <json-c/json.h>

#define BUFFER_DIM 1024
// #define BUFFER_FLAG_DIM 6

static MYSQL* connection;
static pthread_t thread_pool[20];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// ------------------------------ SERVER RELATED FUNCTIONS ------------------------------ //
void* connection_handler(void* socket_desc) {
    int new_socket = (*(int*) socket_desc);
    char* ack_message = "OK";
    char buffer_json_msg[BUFFER_DIM];
    // char flag_buffer[BUFFER_FLAG_DIM];

    memset(&buffer_json_msg, 0, sizeof(buffer_json_msg));
    // memset(&flag_buffer, 0, sizeof(flag_buffer));
    
    bool stop = false;
    while(!stop) {
        // printf("\n    [+ +] Server's thread is now waiting.\n");

        read(new_socket, buffer_json_msg, BUFFER_DIM);
        buffer_json_msg[BUFFER_DIM - 1] = '\0';

        struct json_object* parsed_json = json_tokener_parse(buffer_json_msg);
        struct json_object* flag;
        json_object_object_get_ex(parsed_json, "flag", &flag);
        // fprintf(stdout, "  FLAG: %s\n", json_object_get_string(flag));

        const char* myflag = json_object_get_string(flag);
        if (strcmp(myflag, "LOGIN") == 0) {
            fprintf(stdout, "      [+ + +] Client has requested to login.\n");

            struct json_object* email;
            struct json_object* pass;

            json_object_object_get_ex(parsed_json, "email", &email);
            json_object_object_get_ex(parsed_json, "password", &pass);

            fprintf(stdout, "  EMAIL: %s\n", json_object_get_string(email));
            fprintf(stdout, "  PASSWORD: %s\n", json_object_get_string(pass));
        }
/*
        if (strcmp(flag_buffer, "LOGIN") == 0) {
            sleep(1);
            write(new_socket, ack_message, strlen(ack_message));
            printf("      [+ + +] OK sent to client.\n");

            // read(new_socket, buffer_msg, BUFFER_DIM);
            // printf("      [+ + +] READ: %s\n", buffer_msg);
        } else if (strcmp(flag_buffer, "STOP_C") == 0) { stop = true; }
*/ 
        stop = true;
    }

    printf("\n[+] Terminating connection with client: closing socket.\n");
    close(new_socket);
    return EXIT_SUCCESS;
}

void launch(server_t* server) {
    connection = init_mysql_connection(connection, util_read_password_from_file());
    int i = 0;
    
    printf("[+] MySQL connection successful.\n");
    printf("[+] Server is now waiting for connections.\n");

    // MULTI-CLIENT SERVER
    while(1) {
        socklen_t address_len = sizeof(server->address);
        int new_socket = accept(server->socket, (struct sockaddr*) &server->address, &address_len);
        
        if (new_socket < 0) {
            perror("[-] Could not accept client connection: take a look at client's terminal.");
            exit(1);
        }
        printf("\n[+] Client connection accepted.\n");
        if (pthread_create(&thread_pool[i++], NULL, connection_handler, (void*) &new_socket) < 0) {
            perror("[-] Could not create thread.");
            exit(1);
        }
		printf("    [+ +] Thread created for client requests.\n");
        
        if (i >= 10) {
            i = 0;
            while (i < 10)
                pthread_join(thread_pool[i++], NULL);
            i = 0;
        }
    }
}

// ------------------------------ MYSQL RELATED FUNCTIONS ------------------------------ //
MYSQL* init_mysql_connection(MYSQL* connection, char* password) {
    connection = mysql_init(NULL);
    if (!mysql_real_connect(connection, server, user, password, database, 0, NULL, 0)) {
            fprintf(stderr, "%s\n", mysql_error(connection));
            exit(1);
    }
    return connection;
}

MYSQL_RES* make_query_get_result(MYSQL* connection, char* query) {
	if (mysql_query(connection, query)) {
		fprintf(stderr, "%s\n", mysql_error(connection));
		exit(1);
	}
	MYSQL_RES* result = mysql_use_result(connection);
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
        fprintf(stdout, "%d. ['%s']\n", step, row[0]);
    }
	mysql_free_result(result);
}

// ------------------------------ MAIN  ------------------------------ //
int main(int argc, char** argv) {
    pthread_mutex_init(&lock, NULL);

    server_t server = create_server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 6969, 10);
    launch(&server);
    
    // MYSQL_ROW row;
    // char* query = "select * from users";
    // make_query_print_result(connection, query);
    
	mysql_close(connection); // DON'T FORGET TO CLOSE MYSQL CONNECTION BEFORE ENDING THE PROGRAM
    pthread_mutex_destroy(&lock);

    printf("PROGRAM ENDED STATUS [ OK ]\n");
    return 0;
}