#include "../include/main.h"
#include "../include/server.h"

#define UTILS_H_IMPLEMENTATION
#include "../include/utils.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_DIM 512
#define MAX_CLIENTS 10

MYSQL* connection;
pthread_t thread_pool[20];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// ------------------------------- SERVER RELATED FUNCTIONS ------------------------------- //
void make_query_send_json(int new_socket, MYSQL* connection, char query[], char* flag) {
    pthread_mutex_lock(&lock);
    if (mysql_query(connection, query)) {
		fprintf(stderr, "%s\n", mysql_error(connection));
		exit(1);
	}

	MYSQL_RES* result = mysql_store_result(connection);
    if (result == NULL) {
        fprintf(stderr, "make_query_send_json() failed.\n");
        exit(1);
    }
    send_generated_json(new_socket, result, flag);
    pthread_mutex_unlock(&lock);
}

void send_generated_json(int new_socket, MYSQL_RES* result, char* flag) {
    MYSQL_ROW row;
    int num_fields = mysql_num_fields(result);
    json_object* jobj = json_object_new_array();

    while ((row = mysql_fetch_row(result))) {
        json_object* jrow = json_object_new_object();
        for (int i = 0; i < num_fields; i++) {
            json_object* jval = json_object_new_string(row[i]);
            MYSQL_FIELD* field = mysql_fetch_field_direct(result, i);
            json_object_object_add(jrow, field->name, jval);
        }
        json_object_array_add(jobj, jrow);
    }

    json_object* jwrapper = json_object_new_object();
    json_object* jflag = json_object_new_string(flag);
    json_object_object_add(jwrapper, "flag", jflag);
    json_object_object_add(jwrapper, "retrieved_data", jobj);

    const char* json_str = json_object_to_json_string(jwrapper);
    if (write(new_socket, json_str, strlen(json_str)) < 0) {
        perror("      [- - -] Failed to send json to the client.\n");
        exit(1);
    }
    json_object_put(jwrapper);
}

void send_failure_json(int new_socket, const char* flag) {
    // RETURNS A JSON OBJECT LIKE {flag: "FAILURE", retrieved_data: []}
    json_object* root = json_object_new_object();
    json_object_object_add(root, "flag", json_object_new_string(flag));

    json_object* array = json_object_new_array();
    
    json_object* obj = json_object_new_object();
    json_object_array_add(array, obj);

    json_object_object_add(root, "retrieved_data", array);
    const char *json_str = json_object_to_json_string(root);

    if (write(new_socket, json_str, strlen(json_str)) < 0) {
        perror("      [- - -] Failed to send json to the client.\n");
        exit(1);
    }
    json_object_put(root);
}

void send_success_json_empty_body(int new_socket) {
    json_object* root = json_object_new_object();
    json_object_object_add(root, "flag", json_object_new_string("SUCCESS"));

    json_object* array = json_object_new_array();
    
    json_object* obj = json_object_new_object();
    json_object_array_add(array, obj);

    json_object_object_add(root, "retrieved_data", array);
    const char *json_str = json_object_to_json_string(root);

    if (write(new_socket, json_str, strlen(json_str)) < 0) {
        perror("      [- - -] Failed to send json to the client.\n");
        exit(1);
    }
    json_object_put(root);
}

void* connection_handler(void* socket_desc) {
    int new_socket = (*(int*) socket_desc);
    char buffer_json_msg[BUFFER_DIM];

    memset(&buffer_json_msg, 0, sizeof(buffer_json_msg));
    bool stop = false;
    while(!stop) {
        buffer_json_msg[0] = '\0';
        int bytes_received = read(new_socket, buffer_json_msg, BUFFER_DIM);

        buffer_json_msg[BUFFER_DIM] = '\0';
        struct json_object* parsed_json = json_tokener_parse(buffer_json_msg);
        struct json_object* flag;
        json_object_object_get_ex(parsed_json, "flag", &flag);
        const char* myflag = json_object_get_string(flag);

        if ((bytes_received <= 0) || (strcmp(myflag, "STOP_CONNECTION") == 0)) {
            printf("\n[+] Client interrupted: client socket has been closed.\n");
            stop = true;
            close(new_socket);
        } else {
            if (strcmp(myflag, "LOGIN") == 0) {
                manage_login(new_socket, parsed_json, connection, lock);
            } else if (strcmp(myflag, "REGISTER") == 0) {
                manage_register(new_socket, parsed_json, connection, lock);
            } else if (strcmp(myflag, "FRGTPASS") == 0) {
                manage_forgot_password(new_socket, parsed_json, connection, lock);
            } else if (strcmp(myflag, "NEW_PASSWORD") == 0) {
                manage_alter_password(new_socket, parsed_json, connection, lock);
            } else if (strcmp(myflag, "GET_TICKET") == 0) {
                manage_get_ticket(new_socket, parsed_json, connection, lock);
            }
        }
    }

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
            close(new_socket);
            exit(1);
        }
		printf("    [+ +] Thread created for client requests.\n");
        
        if (i >= MAX_CLIENTS) {
            i = 0;
            while (i < MAX_CLIENTS)
                pthread_join(thread_pool[i++], NULL);
            i = 0;
        }
    }
    close(server->socket);
}

MYSQL* init_mysql_connection(MYSQL* connection, char* password) {
    connection = mysql_init(NULL);
    if (!mysql_real_connect(connection, server, user, password, database, 0, NULL, 0)) {
            fprintf(stderr, "%s\n", mysql_error(connection));
            exit(1);
    }
    return connection;
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