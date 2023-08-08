# Developing a TCP Server Socket

This is an implementation of a TCP server socket, all written in C language. <br />
To breefly explain what the server does, basically it handles connections from clients that need to be interfaced with a DBMS, in this case MySQL. <br />
    1. The client sends to the server its data so that the server can perform queries. <br />
    2. The server sends back to the client the results he retrieved.
<br /> The motivation that led to the development of this project was the need to create a client-server application for museum users.
<br /> The development process of this project took place in a Linux environment, Ubuntu20.04 to be specific.

## MySQL get started tutorial on Ubuntu20.04

### Install MySQL
```console
$ sudo apt update
$ sudo apt install mysql-server && mysql --version
```

### Start MySQL server, configure security script and Login as root user
```console
$ sudo /etc/init.d/mysql start
$ sudo mysql_secure_installation
$ sudo /etc/init.d/mysql stop && sudo /etc/init.d/mysql start
$ mysql -u root -p
```

### Once finished, remember:
```console
$ sudo /etc/init.d/mysql stop
```

## About folders

/include contains all header files. <br />
/src contains all source files. <br />
/bin contains the executable file. <br />
/mysql contains bash scripts used to interact with the DBMS. <br />

## Test the server

### Compiling the code
```console
$ chmod +x build.sh
$ ./build.sh
```

### Execute the code
```console
$ cd bin
$ ./main
```

## License

[MIT](https://choosealicense.com/licenses/mit/)