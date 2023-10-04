# Project: Build a TCP Server Socket

## Overview
What led me to develop a project like this was the need to create a client-server application for museum users. <br />
Since you (user) are not in possession of the database I used and with which the server interfaces, you can't run the application. <br />
At this point, you may wonder why I am publishing this code in the first place: I want you to learn from this. <br />
Use this project as an example of how to interface yourself with: <br />
*Tabspace* 1. Socket programming <br />
*Tabspace* 2. <mysql.h> <br />
*Tabspace* 3. <json-c/json.h> <br /> <br />

Just two more things:
*Tabspace* * The entire API is defined in include/server.h <br />
*Tabspace* * In include/utils.h you can find all utilities (like database name, db user name, etc...) <br />
The development process of this project took place in a Linux environment, to be specific: Ubuntu20.04.6 and Ubuntu22.04.3. <br />

## Get started with MySQL on Ubuntu

### Install MySQL
```bash
sudo apt update
sudo apt install mysql-server && mysql --version
```

### Start MySQL server, configure security script and Login as root user
```bash
sudo /etc/init.d/mysql start
sudo mysql
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY ' '; # Then quit MySQL
sudo mysql_secure_installation
sudo /etc/init.d/mysql stop && sudo /etc/init.d/mysql start
mysql -u root -p
```

### Before shutting down your Ubuntu machine, remember:
```bash
sudo /etc/init.d/mysql stop
```

## About folders

/include contains all header files. <br />
/src contains all source files. <br />
/bin contains the executable file. <br />

### How I compile and execute the project
```bash
sudo /etc/init.d/mysql start
chmod +x build.sh
./build.sh
cd bin
./main
```

## License

[MIT](https://choosealicense.com/licenses/mit/)