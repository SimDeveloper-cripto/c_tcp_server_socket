gcc -pthread -o main $(mysql_config --cflags) src/server.c src/main.c $(mysql_config --libs)