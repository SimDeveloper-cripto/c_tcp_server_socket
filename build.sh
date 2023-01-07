#!/bin/sh

# If your program uses the library <json-c/json.h>
# gcc -pthread -ljson-c -o bin/main $(mysql_config --cflags) src/list.c src/server.c src/main.c $(mysql_config --libs)

gcc -pthread -o bin/main $(mysql_config --cflags) src/list.c src/server.c src/main.c $(mysql_config --libs)
echo "[+] The program has been compiled successfully!\n[+] ./main executable generated in /bin folder."