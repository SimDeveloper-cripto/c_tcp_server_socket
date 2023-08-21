#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <stdio.h>
#include <string.h>

#define BUZZ_SIZE 1024

// HERE WE LIST MYSQL CONNECTION VARIABLES
char* server   = "localhost";
char* user     = "root";        /* Remeber to change this value. */
char* database = "history4fun"; /* Remeber to change this value. */
static char mysql_password_buffer[BUZZ_SIZE];

char* util_read_password_from_file();

#endif /* UTILS_H */

#ifdef UTILS_H_IMPLEMENTATION

// RETRIEVE MYSQL USER PASSWORD FROM A FILE
char* util_read_password_from_file() {
    FILE* f;
    
    if ((f = fopen("../password.txt", "r")) == NULL) {
        perror("[-] Failed to read password from file (MySQL password).\n");
        exit(1);
    }

    fgets(mysql_password_buffer, BUZZ_SIZE, f);
    fclose(f);
    return &mysql_password_buffer[0];
}

// ...

#endif /* UTILS_H */