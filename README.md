# A Simple File Transfer Protocol program

## About program
This is a simple network protocol that can be used to download files from a remote site and to upload files to a remote site, and a client and a server programs that communicate using that protocol. The protocol use TCP as its transport layer protocol. Server can able to serve multiple client requests simultaneously and server will provide its service to any client with the right site address and port number. 

Here are the functions that File Transfer Protocol can perform

1. **pwd** – Function can show the current working directory of the server and send it back to client.

2. **lpwd** – Function can show the current working directory of the client.

3. **cd directory_path** - Function can change the working directory of the server that is serving client and it can support all the path such as “.” and “..”.Also function isable to change toserver root (home folder) when user enters cd without parameter and will show the error message when the wrong directory path is being entered.

4. **lcd directory_path** - Function can change the working directory of the client and it can support all the directory path such as “.” and “..”.Also function is able to change to  client root (home folder) when user enters cd without parameter and will show the error message when the wrong directory path is being entered. 

5.	**dir** – function can show all the lists of the file and folders that is under the current working directory of the server.

6.	**ldir** – function can show all the lists of the files and folders that is under the current working directory of the server

7. **put filename** - Function can upload the named file from the current directory of the client to the current directory of the remove server. 

8. **get filename**  - Function to download the named file from the current directory of the remote server and save it in the current directory of the client;

## How to run the program

This program is well-suited to test out on the server but however, for those who don't have servers, here is the way to use the program.

**Step 1: Requirements**

You need to have 2 machines, one machine is to perform as the server and another is to perform as the client. You can use the virtual machine to make 2 machine. 

**Step 2: Setting up the server** 

Firstly, in order to set up the server, download the folder called "Server" and put onto the machine that will perform as the server. Then, open the command line and locate the directory path that folder "Server" is existed on your machine. After that, type "make" to complie and run the makefile to compile the files. After compiling the file, enter "./myftpd" to start running the server. If the program shows Server PID, it can be said that server is running without any errors and is waiting for the client connection to establish connections.

**Step 3: Setting up the client** 

To set up the client, download the folder called "Client" and put onto the machine that will perform as the server. Then, open the command line and locate the directory path that folder "Client" is existed on your machine. After that, type "make" to complie and run the makefile to compile the files. After compiling the file, enter "./myftp <server IP address>" to start connecting to the server. You will see lists of functions on the client side and a successful connection establishment message on the server side and this is being said, establishing connection between server and client is successful. 

