#ifndef MAIN_H
#define MAIN_H

#include <mysql.h>
#include <stdio.h>

// SERVER'S HELPER LIBRARY

// ...

/* FUNCTIONS */
MYSQL* init_mysql_connection(MYSQL* connection, char* password);  // RETURNS THE CONNECTION INSTANCE
void make_query_send_json(int new_socket, MYSQL* connection, char query[], char* flag);
void send_generated_json(int new_socket, MYSQL_RES* result, char* flag);
void send_failure_json(int new_socket, const char* flag);
void send_success_json_empty_body(int new_socket);

// ...

#endif /* MAIN_H */