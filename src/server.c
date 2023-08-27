#include "../include/server.h"
#include "../include/main.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

server_t create_server(int address_family, int service, int protocol, u_long interface, int port, int backlog) {
    int opt = 1;
    server_t server;

    server.address_family = address_family;
    server.service        = service;
    server.protocol       = protocol;
    server.interface      = interface;
    server.port           = port;
    server.backlog        = backlog;
    server.address.sin_family      = address_family;
    server.address.sin_port        = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.socket = socket(server.address_family, server.service, server.protocol);
    if (server.socket < 0) {
        perror("[-] Socket failed.\n");
        exit(1);
    }
    puts("[+] Server socket created.");

    if (setsockopt(server.socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("[-] setsockopt failed.\n");
        exit(1);
    }

    if (bind(server.socket, (struct sockaddr*) &server.address, sizeof(server.address)) < 0) {
        perror("[-] Socket bind failed.");
        exit(1);
    }
    puts("[+] Socket bind successful.");

    if (listen(server.socket, server.backlog) < 0) {
        perror("[-] Failed to start listening.\n");
        exit(1);
    }

    printf("[+] Sever is up at 'localhost:%d'\n", server.port);

    return server;
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

bool exists(MYSQL* connection, char query[], pthread_mutex_t lock) {
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

void manage_retrieve_opera_descriptions(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock){
    struct json_object* json_area;
    struct json_object* json_description;

    json_object_object_get_ex(parsed_json, "area", &json_area);
    json_object_object_get_ex(parsed_json, "description", &json_description);
    
    const char* area        = json_object_get_string(json_area);
    const char* description = json_object_get_string(json_description);
    
    char return_query[256];
    snprintf(return_query, sizeof(return_query), "SELECT artifact_id,%s FROM artifacts WHERE area='%s' ORDER BY artifact_id", description, area);
    make_query_send_json(new_socket, connection, return_query, "SUCCESS");
}

void manage_retrieve_ticket_type(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock) {
    struct json_object* json_user_id;
    struct json_object* json_ticket_date;

    json_object_object_get_ex(parsed_json, "user_id", &json_user_id);
    json_object_object_get_ex(parsed_json, "ticket_date", &json_ticket_date);
    
    const char* user_id       = json_object_get_string(json_user_id);
    const char* ticket_date   = json_object_get_string(json_ticket_date);
    
    char return_query[256];
    snprintf(return_query, sizeof(return_query), "SELECT type FROM tickets WHERE user_id='%s' AND ticket_date='%s'", user_id, ticket_date);
    make_query_send_json(new_socket, connection, return_query, "SUCCESS");
}

void manage_check_ticket_acquired(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock) {
    struct json_object* json_user_id;
    struct json_object* json_ticket_date;
    struct json_object* json_area;

    json_object_object_get_ex(parsed_json, "user_id", &json_user_id);
    json_object_object_get_ex(parsed_json, "ticket_date", &json_ticket_date);
    json_object_object_get_ex(parsed_json, "area", &json_area);

    const char* user_id       = json_object_get_string(json_user_id);
    const char* ticket_date   = json_object_get_string(json_ticket_date);
    const char* area          = json_object_get_string(json_area);
    
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM tickets WHERE user_id='%s' AND ticket_date='%s' AND area='%s'", user_id, ticket_date, area);
    if (!exists(connection, query, lock)) {
        send_failure_json(new_socket, "FAILURE");
    } else {
        char return_query[256];

        // HERE WE RETURN JUST THE AREA CHOSEN BY THE USERS FOR THAT SPECIFIC DAY
        // TO GET ALL THE DESCRIPTIONS WE USE ANOTHER FLAG
        snprintf(return_query, sizeof(return_query), "SELECT area FROM tickets WHERE user_id='%s' AND ticket_date='%s'", user_id, ticket_date);
        make_query_send_json(new_socket, connection, return_query, "SUCCESS");
    }
}

void manage_get_ticket(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock) {
    struct json_object* json_ticket_id;
    struct json_object* json_user_id;
    struct json_object* json_n_followers;
    struct json_object* json_ticket_date;
    struct json_object* json_type;
    struct json_object* json_cost;
    struct json_object* json_area;

    json_object_object_get_ex(parsed_json, "ticket_id", &json_ticket_id);
    json_object_object_get_ex(parsed_json, "user_id", &json_user_id);
    json_object_object_get_ex(parsed_json, "n_followers", &json_n_followers);
    json_object_object_get_ex(parsed_json, "ticket_date", &json_ticket_date);
    json_object_object_get_ex(parsed_json, "type", &json_type);
    json_object_object_get_ex(parsed_json, "cost", &json_cost);
    json_object_object_get_ex(parsed_json, "area", &json_area);

    const char* ticket_id    = json_object_get_string(json_ticket_id);
    const char* user_id       = json_object_get_string(json_user_id);
    const int32_t n_followers = json_object_get_int(json_n_followers);
    const char* ticket_date   = json_object_get_string(json_ticket_date);
    const char* type          = json_object_get_string(json_type);

    double cost              = json_object_get_double(json_cost);
    float cost_value         = (float) cost;

    const char* area          = json_object_get_string(json_area);

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM tickets WHERE ticket_id='%s'", ticket_id);
    if (!exists(connection, query, lock)) {
        char second_query[256];
        snprintf(second_query, sizeof(second_query), "SELECT * FROM tickets WHERE user_id='%s' AND ticket_date='%s'", user_id, ticket_date);
        if (!exists(connection, second_query, lock)) {
            char query_insert[1024];
            snprintf(query_insert, sizeof(query_insert), "INSERT INTO tickets (ticket_id, user_id, n_followers, ticket_date, type, cost, area) VALUES ('%s', '%s', '%d', '%s', '%s', '%f', '%s')", 
                ticket_id, user_id, n_followers, ticket_date, type, cost_value, area);

            pthread_mutex_lock(&lock);
            if (mysql_real_query(connection, query_insert, strlen(query_insert))) {
                printf("[ERROR] INSERT failed: %s\n", mysql_error(connection));
                exit(1);
            }
            pthread_mutex_unlock(&lock);

            char return_query[256];
            snprintf(return_query, sizeof(return_query), "SELECT ticket_id,cost FROM tickets WHERE user_id='%s' AND ticket_date='%s'", user_id, ticket_date);
            make_query_send_json(new_socket, connection, return_query, "SUCCESS");
        } else {
            send_failure_json(new_socket, "ALREADY_EXISTS");
        }
    } else {
        // QUESTO ERRORE PUO' ESSERE GENERATO SOLO NEL CASO DAVVERO IMPROBABILE CHE VENGANO GENERATI DUE TICKET_CODES UGUALI
        send_failure_json(new_socket, "FAILURE");
    }

    return;
}

void manage_alter_password(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock) {
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
        send_failure_json(new_socket, "FAILURE");
    } else {
        send_success_json_empty_body(new_socket);
    }
    pthread_mutex_unlock(&lock);
}

void manage_forgot_password(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock) {
    // 1. MUST CHECK IF THE E-MAIL ALREADY EXISTS
    struct json_object* json_email;
    json_object_object_get_ex(parsed_json, "email", &json_email);
    const char* email = json_object_get_string(json_email);

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE email='%s'", email);
    if (exists(connection, query, lock)) {
        // 2. IF "SUCCESS", SEND TO THE CLIENT THE JSON BODY WITH THE RELATIVE UNIQUE CODE
        send_random_code(new_socket);
    } else {
        // 3. HANDLE "FAILURE" RESPONSE
        send_failure_json(new_socket, "FAILURE");
    }
    return;
}

void manage_register(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock) {
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
    if (!exists(connection, query, lock)) {
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
        send_failure_json(new_socket, "FAILURE");
    }
    return;
}

void manage_login(int new_socket, struct json_object* parsed_json, MYSQL* connection, pthread_mutex_t lock) {
    struct json_object* json_email;
    struct json_object* json_pass;

    json_object_object_get_ex(parsed_json, "email", &json_email);
    json_object_object_get_ex(parsed_json, "password", &json_pass);

    const char* email = json_object_get_string(json_email);
    const char* pass = json_object_get_string(json_pass);

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE email='%s' AND password='%s'", email, pass);
    if (exists(connection, query, lock)) {
        make_query_send_json(new_socket, connection, query, "SUCCESS");
    } else {
        send_failure_json(new_socket, "FAILURE");
    }
    return;
}