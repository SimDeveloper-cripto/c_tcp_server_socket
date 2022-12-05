#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#define BUZZ_SIZE 1024

char* util_read_password_from_file();

#endif /* UTILS_H */

#ifdef UTILS_H_IMPLEMENTATION

// RETRIEVE MYSQL USER PASSWORD FROM A FILE
char* util_read_password_from_file() {
    char* buff = malloc(1024);
    FILE* f = fopen("../password.txt", "r");
    fgets(buff, BUZZ_SIZE, f);
    fclose(f);
    return buff;
}

#endif