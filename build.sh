#!/bin/sh
gcc -Wall -pthread -o bin/main $(mysql_config --cflags) src/list.c src/server.c src/main.c $(mysql_config --libs)
echo "[+] The program has been compiled successfully!\n[+] ./main executable generated in /bin folder."

# Use this flag if defined header <json-c/json.h> '-ljson-c'