#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <mysql.h>

// HERE WE LIST MYSQL CONNECTION VARIABLES
char* server = "localhost";
char* user = "simone";          /* Remeber to change this value. */
char* database = "mytest";      /* Remeber to change this value. */
// char* password = "";            /* Remeber to change this value. */

// FUNCTIONS

MYSQL* init_mysql_connection(MYSQL* connection, char* password);  // RETURNS THE CONNECTION INSTANCE
void make_query_print_result(MYSQL* connection, char* query);
MYSQL_RES* make_query_get_result(MYSQL* connection, char* query); // AFTER CALLING THIS FUNCTION, ALSO CALL 'mysql_free_result( <MYSQL_RES*> );'

#endif /* MAIN_H */