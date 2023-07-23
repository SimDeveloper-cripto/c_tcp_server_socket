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
#define MAX_CLIENTS 10

static MYSQL* connection;
static pthread_t thread_pool[20];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// ------------------------------- SERVER RELATED FUNCTIONS ------------------------------- //
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

void send_failure_json(int new_socket) {
    // RETURNS A JSON OBJECT LIKE {flag: "FAILURE", retrieved_data: []}
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

void send_random_code(int new_socket) {
    srand(time(NULL));
    char random_code[6];
    for (size_t i = 0; i < 5; i++) {
        int num = rand() % 10;
        random_code[i] = num + '0';
    }
    random_code[5] = '\0';

    json_object *flag = json_object_new_string("SUCCESS");
    json_object *data_array = json_object_new_array();
    json_object *data_string = json_object_new_string(random_code);
    json_object_array_add(data_array, data_string);
    json_object *retrieved_data = json_object_new_object();
    json_object_object_add(retrieved_data, "code", data_array);
    json_object *json = json_object_new_object();
    json_object_object_add(json, "flag", flag);
    json_object_object_add(json, "retrieved_data", retrieved_data);

    const char *json_string = json_object_to_json_string(json);
    if (write(new_socket, json_string, strlen(json_string)) < 0) {
        perror("      [- - -] Failed to send json to the client.\n");
        exit(1);
    }
    json_object_put(json);
    return;
}

void manage_alter_password(int new_socket, struct json_object* parsed_json) {
    struct json_object* json_new_pass;
    struct json_object* json_email;

    json_object_object_get_ex(parsed_json, "new_password", &json_new_pass);
    json_object_object_get_ex(parsed_json, "email", &json_email);

    const char* new_pass = json_object_get_string(json_new_pass);
    const char* _email   = json_object_get_string(json_email);

    char query[256];
    snprintf(query, sizeof(query), "UPDATE users SET password='%s' WHERE email='%s'", new_pass, _email);
        
    pthread_mutex_lock(&lock);
    if (mysql_real_query(connection, query, strlen(query))) {
        send_failure_json(new_socket);
    } else {
        send_success_json_empty_body(new_socket);
    }
    pthread_mutex_unlock(&lock);
}

void manage_forgot_password(int new_socket, struct json_object* parsed_json) {
    // 1. MUST CHECK IF THE E-MAIL ALREADY EXISTS
    struct json_object* json_email;
    json_object_object_get_ex(parsed_json, "email", &json_email);
    const char* email = json_object_get_string(json_email);

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE email='%s'", email);
    if (exists(connection, query)) {
        // 2. IF "SUCCESS", SEND TO THE CLIENT THE JSON BODY WITH THE RELATIVE UNIQUE CODE
        send_random_code(new_socket);
    } else {
        // 3. HANDLE "FAILURE" RESPONSE
        send_failure_json(new_socket);
    }
    return;
}

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
        const int32_t eta       = json_object_get_int(json_age);
        const int32_t expert    = json_object_get_int(json_expert);

        char query[512];
        snprintf(query, sizeof(query), "INSERT INTO users (user_id, name, surname, email, password, age, phone_number, expert) VALUES ('%s', '%s', '%s', '%s', '%s', '%d', '%s', '%d')", 
            u_id, name, surname, email, pass, eta, telefono, expert);
        
        pthread_mutex_lock(&lock);
        if (mysql_real_query(connection, query, strlen(query))) {
            printf("[ERROR] INSERT failed: %s\n", mysql_error(connection));
            exit(1);
        }
        pthread_mutex_unlock(&lock);

        char query2[256];
        snprintf(query2, sizeof(query), "SELECT * FROM users WHERE user_id='%s'", u_id);
        make_query_send_json(new_socket, connection, query2, "SUCCESS");
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
                manage_login(new_socket, parsed_json);
            } else if (strcmp(myflag, "REGISTER") == 0) {
                manage_register(new_socket, parsed_json);
            } else if (strcmp(myflag, "FRGTPASS") == 0) {
                manage_forgot_password(new_socket, parsed_json);
            } else if (strcmp(myflag, "NEW_PASSWORD") == 0) {
                manage_alter_password(new_socket, parsed_json);
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

// ------------------------------ MYSQL RELATED FUNCTIONS ------------------------------ //
MYSQL* init_mysql_connection(MYSQL* connection, char* password) {
    connection = mysql_init(NULL);
    if (!mysql_real_connect(connection, server, user, password, database, 0, NULL, 0)) {
            fprintf(stderr, "%s\n", mysql_error(connection));
            exit(1);
    }
    return connection;
}

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