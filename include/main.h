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
void make_query_print_result(MYSQL* connection, char query[]);
bool exists(MYSQL* connection, char query[]);

// UTILITY FUNCTIONS
void manage_login(struct json_object* parsed_json);

#endif /* MAIN_H */