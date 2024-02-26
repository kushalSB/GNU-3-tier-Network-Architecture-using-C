# Three files are here

- The client file is a GNU based GUI program that is multiplatform built using gtk3.
- For Modularity , two seperate files are created, change in implementation of simpleMysql file should not affect server file
- simpleMysql can be replaced by other databases according to need, the underlying architecture must remain constant.

## client

- Gtk3 is used to build GUI interface.
- API like structre is implemented to connect with the server and display fetched data

## server

- Server is created using the socket function, it takes three arguments
  - Domain - it specifies communication domain or address family. Here we have use IPv4 address family
  - Type - it specifies communication semantics. Here we have used connection oriented TCP.
  - Protocol- This specifies the specific protocol to be used with socket. Here we have used 0, to let system choose appropriate protocol based on domain and type.

```cpp

server_fd = socket(AF_INET, SOCK_STREAM, 0);
```

- Next we have set socket options using setsocketopt() function. It takes 5 arguments
  - sockfd- The socket descriptor. Here we have used server_fd that we created using socket.
  - Level - The level at which the option resides. Here we want to customize at socket level.
  - optname- This is the name of the option we want to set. The options we can use here depend on the level we have specified.
    - SO_REUSEADDR and SO_REUSEPORT is used to allow socket to reuse local address and port.
  - optval: This is a pointer to the value you want to set for the option specified by optname. It's a void\* because the actual type and size of the option value can vary depending on the option.
  - optlen: This specifies the size of the option value pointed to by optval.

```cpp
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
```

- The next thing we do is bind the socket to a specified port.It takes three arguments.
  - sockfd
  - socket address
  - size of address

```cpp
bind(server_fd, (struct sockaddr *)&address, sizeof(address);
```

- Now we set the server to finally listen

```cpp
listen(server_fd, MAX_CLIENTS);
```

- The connection to client is accepted using accept function.
  - It creates a new file descriptor for this socket.

```cpp
new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)
```

- The server reads values sent from client using read() function. The recieved data is stored in buffer variable.

```cpp
char buffer[1024] = {0};
valread = read(new_socket, buffer, 1024);
printf("Received request from client: %s\n", buffer);
```

- The server sends values to client using send() function.

```cpp
    send(new_socket, "Server: Command Recieved", strlen("Server: Command Recieved"), 0);
```

- In our implementation based on the info on buffer the following things are done.

  - GETALL Returns all entry in table
  - GETONE Returns one entry of the specified id
  - VIEWCOLUMNS displays the column names of tables
  - ADDONE adds one entry to the table (Do not supply id)

- We have implemented a custom function execute_with_pipe() which takes one argument. The argument is a custom string which is expected by simpleMysql file

<p> It uses exec() to run simpleMysql as a child process and reads the STDOUT and stores it in an array. </p>

## simpleMysql

- Connection to MySQL database is established here.
- Query to MySQL is done here, results extracted and printed.
- The printed results are read by dataBaseCommand file
- Lifecycle if for one operation only.
