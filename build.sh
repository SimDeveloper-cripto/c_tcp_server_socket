#!/bin/sh
gcc -pthread -o bin/main $(mysql_config --cflags) src/server.c src/main.c $(mysql_config --libs)