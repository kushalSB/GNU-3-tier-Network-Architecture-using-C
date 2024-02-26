# 3 tiered Network Architecture using C

- More information inside specific folders

# Three folders are here

## console based db for Db operation using console

- This is to create database operation layer and test its working.
- IPC is used here
- Pipes are implemented
- The STDOUT of child is read by the parent process and stored in a buffer.
- This buffer is then printed out by the parent process.
- The child process runs another executable that uses MySQL ODBC to make changes to database.

## console based server for client-server-dboperation (3 tiered netword strucutre)

- In addition to database operation. A console based client is implemented here.
- It is a complete implementation of 3 tiered layer, with server also acting as middleware.

## gui based (using gtk) client-server-dboperation (3 tiered network structure)(API- like)

- GTK3 is used to create Client.
- Server is redesigned to handle client requests and send appropriate response
