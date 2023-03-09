#include "../include/list.h"
#include "../include/main.h"
#include "../include/server.h"

#define UTILS_H_IMPLEMENTATION
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <json-c/json.h>

#define BUFFER_DIM 512

static MYSQL* connection;
static pthread_t thread_pool[20];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// ------------------------------ SERVER RELATED FUNCTIONS ------------------------------ //
void manage_login(int new_socket, struct json_object* parsed_json) {
    struct json_object* json_email;
    struct json_object* json_pass;

    json_object_object_get_ex(parsed_json, "email", &json_email);
    json_object_object_get_ex(parsed_json, "password", &json_pass);

    const char* email = json_object_get_string(json_email);
    const char* pass = json_object_get_string(json_pass);
    char query[256];

    snprintf(query, sizeof(query), "SELECT * FROM users WHERE email='%s' AND password='%s'", email, pass);
    if (exists(connection, query)) {
        make_query_send_json(new_socket, connection, query, "SUCCESS");
    } else {
        send_failure_json(new_socket);
    }
    return;
}

void manage_register(int new_socket, struct json_object* parsed_json) {
    struct json_object* json_user_id;
    struct json_object* json_name;
    struct json_object* json_surname;
    struct json_object* json_email;
    struct json_object* json_password;
    struct json_object* json_age;
    struct json_object* json_phone;
    struct json_object* json_expert;

    json_object_object_get_ex(parsed_json, "user_id", &json_user_id);
    json_object_object_get_ex(parsed_json, "name", &json_name);
    json_object_object_get_ex(parsed_json, "surname", &json_surname);
    json_object_object_get_ex(parsed_json, "email", &json_email);
    json_object_object_get_ex(parsed_json, "password", &json_password);
    json_object_object_get_ex(parsed_json, "age", &json_age);
    json_object_object_get_ex(parsed_json, "phone_number", &json_phone);
    json_object_object_get_ex(parsed_json, "expert", &json_expert);

    const char* telefono = json_object_get_string(json_phone);
    const char* email    = json_object_get_string(json_email);

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE email='%s' OR phone_number='%s'", email, telefono);
    if (!exists(connection, query)) {
        const char* u_id        = json_object_get_string(json_user_id);
        const char* name        = json_object_get_string(json_name);
        const char* surname     = json_object_get_string(json_surname);
        const char* pass        = json_object_get_string(json_password);
        const int32_t eta      = json_object_get_int(json_age);
        const int32_t expert   = json_object_get_int(json_expert);

        // insert_into_users(connection, u_id, name, surname, email, pass, eta, telefono, expert);

        pthread_mutex_lock(&lock);
        char query[512];
        snprintf(query, sizeof(query), "INSERT INTO users (user_id, name, surname, email, password, age, phone_number, expert) VALUES ('%s', '%s', '%s', '%s', '%s', '%d', '%s', '%d')", 
            u_id, name, surname, email, pass, eta, telefono, expert);
        
        if (mysql_real_query(connection, query, strlen(query))) {
            printf("[ERROR] INSERT failed: %s\n", mysql_error(connection));
            exit(1);
        }
        pthread_mutex_unlock(&lock);
        
        fprintf(stdout, "Utente inserito nel database!\n");
        // TODO: MANDARE JSON SUCCESS CON IL CORPO CORRETTO AL CLIENT
    } else {
        send_failure_json(new_socket);
    }
    return;
}

void* connection_handler(void* socket_desc) {
    int new_socket = (*(int*) socket_desc);
    char buffer_json_msg[BUFFER_DIM];

    memset(&buffer_json_msg, 0, sizeof(buffer_json_msg));
    bool stop = false;
    while(!stop) {
        buffer_json_msg[0] = '\0';
        read(new_socket, buffer_json_msg, BUFFER_DIM);
        
        buffer_json_msg[BUFFER_DIM - 1] = '\0';
        struct json_object* parsed_json = json_tokener_parse(buffer_json_msg);
        struct json_object* flag;
        json_object_object_get_ex(parsed_json, "flag", &flag);

        const char* myflag = json_object_get_string(flag);
        if (strcmp(myflag, "LOGIN") == 0) {
            manage_login(new_socket, parsed_json);
        } else if (strcmp(myflag, "REGISTER") == 0) {
            manage_register(new_socket, parsed_json);
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

// TODO: QUESTE FUNZIONI CON MOLTA PROBABILITA' ANDRANNO MODIFICATE
// TODO: NELLA HOME, L'UTENTE DEVE APPARIRE ANCHE CON IL SUO USER_ID

void insert_into_users(MYSQL* connection, const char* u_id, const char* name, const char* surname, 
    const char* email, const char* pass, const int* eta, const char* telefono, const int* expert) {
    // TODO: COSA FARE DI QUESTA FUNZIONE?
}

void make_query_send_json(int new_socket, MYSQL* connection, char query[], char* flag) {
    pthread_mutex_lock(&lock);
    if (mysql_query(connection, query)) {
		fprintf(stderr, "%s\n", mysql_error(connection));
		return;
	}

	MYSQL_RES* result = mysql_store_result(connection);
    if (result == NULL) {
        fprintf(stderr, "make_query_send_json() failed.\n");
        return;
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

    // const char* json_str = json_object_to_json_string(jobj);
    const char* json_str = json_object_to_json_string(jwrapper);
    if (write(new_socket, json_str, strlen(json_str)) < 0) {
        perror("      [- - -] Failed to send json to the client.\n");
        exit(1);
    }
    // json_object_put(jobj);
    json_object_put(jwrapper);
}

void send_failure_json(int new_socket) {
    json_object* root = json_object_new_object();
    json_object_object_add(root, "flag", json_object_new_string("FAILURE"));

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