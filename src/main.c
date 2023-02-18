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

#define BUFFER_DIM 2048

static MYSQL* connection;
static pthread_t thread_pool[20];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// ------------------------------ SERVER RELATED FUNCTIONS ------------------------------ //
void manage_login(struct json_object* parsed_json) {
    struct json_object* json_email;
    struct json_object* json_pass;
    fprintf(stdout, "      [+ + +] Client has requested to login.\n");
            
    json_object_object_get_ex(parsed_json, "email", &json_email);
    json_object_object_get_ex(parsed_json, "password", &json_pass);

    const char* email = json_object_get_string(json_email);
    const char* pass = json_object_get_string(json_pass);
    char query[256];

    // snprintf(query, sizeof(query), "SELECT COUNT(*) FROM users WHERE email='%s' AND password='%s'", email, pass);
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE email='%s' AND password='%s'", email, pass);
    // make_query_print_result(connection, query);

    if (exists(connection, query)) {
        fprintf(stdout, "      [+ + +] The user exists.\n");
    } else {
        fprintf(stdout, "      [+ + +] The user does not exists.\n");   
    }
}

void* connection_handler(void* socket_desc) {
    int new_socket = (*(int*) socket_desc);
    // char* ack_message = "OK";
    char buffer_json_msg[BUFFER_DIM];

    memset(&buffer_json_msg, 0, sizeof(buffer_json_msg));
    bool stop = false;
    while(!stop) {
        read(new_socket, buffer_json_msg, BUFFER_DIM);
        buffer_json_msg[BUFFER_DIM - 1] = '\0';

        struct json_object* parsed_json = json_tokener_parse(buffer_json_msg);
        struct json_object* flag;
        json_object_object_get_ex(parsed_json, "flag", &flag);

        const char* myflag = json_object_get_string(flag);
        if (strcmp(myflag, "LOGIN") == 0) {
            manage_login(parsed_json);
        } else if (strcmp(myflag, "STOP_CONNECTION") == 0) {
            stop = true;
        }
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

void make_query_print_result(MYSQL* connection, char query[]) {
    pthread_mutex_lock(&lock);
	if (mysql_query(connection, query)) {
		fprintf(stderr, "%s\n", mysql_error(connection));
		return;
	}

    MYSQL_ROW row;
	MYSQL_RES* result = mysql_use_result(connection);
	while ((row = mysql_fetch_row(result)) != NULL) {
        fprintf(stdout, "RESULT: '%s'\n", row[0]);
    }
	mysql_free_result(result);
    pthread_mutex_unlock(&lock);
}

bool exists(MYSQL* connection, char query[]) {
    pthread_mutex_lock(&lock);
	if (mysql_query(connection, query)) {
		fprintf(stderr, "%s\n", mysql_error(connection));
		return false;
	}

	MYSQL_RES* result = mysql_store_result(connection);
    if (result == NULL) return false;
    
    int num_rows = mysql_num_rows(result); 
	mysql_free_result(result);
    pthread_mutex_unlock(&lock);
    return (num_rows > 0);
}

// ------------------------------ MAIN  ------------------------------ //
int main(int argc, char** argv) {
    pthread_mutex_init(&lock, NULL);

    server_t server = create_server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 6969, 10);
    launch(&server);

	mysql_close(connection); // DON'T FORGET TO CLOSE MYSQL CONNECTION BEFORE ENDING THE PROGRAM
    pthread_mutex_destroy(&lock);

    printf("PROGRAM ENDED STATUS [ OK ]\n");
    return 0;
}