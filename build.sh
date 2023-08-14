#!/bin/sh
gcc -Wall -pthread -o bin/main $(mysql_config --cflags) src/server.c src/main.c $(mysql_config --libs) -ljson-c

if [ $? -eq 0 ]; then
    echo "[OK] The program has been compiled successfully!\n[OK] ./main executable generated in /bin folder."
else
    echo "[FAIL] Encountered compilation errors!\n."
fi

# Use this flag if defined header <json-c/json.h> '-ljson-c'