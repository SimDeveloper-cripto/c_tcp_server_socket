#ifndef MAIN_H
#define MAIN_H

#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <json-c/json.h>

// HERE WE LIST MYSQL CONNECTION VARIABLES
char* server = "localhost";
char* user = "root";            /* Remeber to change this value. */
char* database = "history4fun"; /* Remeber to change this value. */

// MYSQL RELATED FUNCTIONS
MYSQL* init_mysql_connection(MYSQL* connection, char* password);  // RETURNS THE CONNECTION INSTANCE
bool exists(MYSQL* connection, char query[]);
void make_query_send_json(int new_socket, MYSQL* connection, char query[], char* flag);
void send_generated_json(int new_socket, MYSQL_RES* result, char* flag);
void send_failure_json(int new_socket);

// UTILITY FUNCTIONS
void manage_login(int new_socket, struct json_object* parsed_json);

#endif /* MAIN_H */