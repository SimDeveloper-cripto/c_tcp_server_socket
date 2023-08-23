#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUZZ_SIZE 1024

// HERE WE LIST MYSQL CONNECTION VARIABLES
char* server   = "localhost";
char* user     = "root";        /* Remeber to change this value. */
char* database = "history4fun"; /* Remeber to change this value. */
static char buffer[BUZZ_SIZE];
const char* file_path = "../password.txt"; 

/* FUNCTIONS */
char* util_read_password_from_file();

#endif /* UTILS_H */

#ifdef UTILS_H_IMPLEMENTATION

/*
    // RETRIEVE MYSQL USER'S PASSWORD FROM A FILE [WE KEEP IT AS BACKUP FUNCTION]
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
*/

char* util_read_password_from_file() {
    int fd = open(file_path, O_RDONLY);
    off_t fileSize = lseek(fd, 0, SEEK_END);
    
    if (fileSize < 0) {
        perror("[-] Could not open file to retrieve user's password.\n");
        close(fd);
        return NULL;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("[-] An error occurred while trying to SEEK_SET.\n");
        close(fd);
        return NULL;
    }

    ssize_t bytesRead = read(fd, buffer, BUZZ_SIZE);
    if (bytesRead < 0) {
        perror("[-] Could not read file.\n");
        close(fd);
        return NULL;
    }

    buffer[fileSize] = '\0';
    close(fd);
    return &buffer[0];
}

// ...

#endif /* UTILS_H */