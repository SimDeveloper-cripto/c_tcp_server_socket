#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define BUZZ_SIZE 1024

// RETRIEVE MYSQL PASSWORD FROM FILE PLEASE
char* util_read_password_from_file() {
    char* buff = malloc(1024);
    FILE* f = fopen("../password.txt", "r");
    fgets(buff, BUZZ_SIZE, f);
    fclose(f);
    return buff;
}

#endif /* UTILS_H */