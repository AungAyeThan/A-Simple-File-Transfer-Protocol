/*
 *  File Name: myftpd.c  
 *  Name : Aung Aye Than and Tan Yeow Keat
 *  1st version: 15th Nov 2017
 *  2nd version: 17th Nov 2017
 *  3rd version: 20th Nov 2017
 *  Final version: 26th Nov 2017
 */
 
#include  <stdlib.h>     /* strlen(), strcmp() etc */
#include  <stdio.h>      /* printf()  */
#include  <string.h>     /* strlen(), strcmp() etc */
#include  <errno.h>      /* extern int errno, EINTR, perror() */
#include  <signal.h>     /* SIGCHLD, sigaction() */
#include  <syslog.h>
#include  <dirent.h>
#include  <sys/types.h>  /* pid_t, u_long, u_short */
#include  <sys/socket.h> /* struct sockaddr, socket(), etc */
#include  <sys/wait.h>   /* waitpid(), WNOHAND */
#include  <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
                         /* and INADDR_ANY */
#include  "stream.h"     /* MAX_BLOCK_SIZE, readn(), writen() */
#include  <unistd.h>
#include  <sys/stat.h>
 
#define   SERV_TCP_PORT   40005   /* default server listening port */


#define EOF_Message 	"30FM5G"
#define fileNameOkay 	"0"//File not found, ready to receive parameter
#define fileNameClash 	"1" //File already exist, file name clash
#define miscError 	"2" //miscellaneous errors such as file opening or anything else 

#define fileAvailable	"0"//File is found 
#define fileUnavailable	"1"//File is not found
#define fileOpenError	"2"//"F1l30P3N3RR0R"//Error opening file

#define cdWithoutParam 	"#"

#define newLogMessage	"=====Server Starting=====\n"
#define endLogMessage	"=====Server Shutdown=====\n"
#define serverLogName	"serverLog.txt"



void claim_children()  //function to claim children
{
     pid_t pid=1;
      
     while (pid>0) { /* claim as many zombies as we can */
         pid = waitpid(0, (int *)0, WNOHANG); 
     }

     FILE* serverLog;//file descriptor for server log
     serverLog = fopen(serverLogName, "a");//open in write mode so it removes previously stored content(reset log)
     fprintf(serverLog, "%s", endLogMessage);
     fclose(serverLog); //close server log
}





void daemon_init(void) //function to make server to run as daemon 
{       
     pid_t   pid;
     struct sigaction act;
 
     if ( (pid = fork()) < 0) {
          perror("fork"); exit(1);  //forking
     } else if (pid > 0) {
          printf("Server PID: %d\n", pid);
          exit(0);                  /* parent goes bye-bye */
     }
 
     /* child continues */
     setsid();                      /* become session leader */
     //chdir("/");                    /* change working directory */
     umask(0);                      /* clear file mode creation mask */
 
     /* catch SIGCHLD to remove zombies from system */
     act.sa_handler = claim_children; /* use reliable signal */
     sigemptyset(&act.sa_mask);       /* not to block other signals */
     act.sa_flags   = SA_NOCLDSTOP;   /* not catch stopped children */
     sigaction(SIGCHLD,(struct sigaction *)&act,(struct sigaction *)0);
     /* note: a less than perfect method is to use 
              signal(SIGCHLD, claim_children);
     */
}



void processpwd(int socket, FILE* serverLog)//to add file descriptor to test server log
{
	char pathName[MAX_BLOCK_SIZE];
    	getcwd(pathName,sizeof(pathName)); //getting the path directory
    	writen(socket, pathName, strlen(pathName)); //writing socket

	serverLog = fopen(serverLogName, "a");//open in append mode so it doesn't previously stored content
	fprintf(serverLog, "called pwd, path name of working directory sent to client.\n");
	fclose(serverLog);
}



void processdir(int socket, FILE* serverLog) //function to process dir function
{

	char location[MAX_BLOCK_SIZE]={0};
	char itemName[MAX_BLOCK_SIZE]={0};
	char itemNameList[MAX_BLOCK_SIZE]={0};
    	getcwd(location,sizeof(location)); //getting current location directory path

        DIR *d; //dir pointer to open directory path
        struct dirent *dir;
        d = opendir(location);//opening current dir

        if(d) //if there is directory
        {
               
                while((dir = readdir(d)) !=NULL)  //condition to check whether there is vaild directory location
                {
			if(strcmp(dir->d_name,".")!=0&&strcmp(dir->d_name,"..")!=0) 
                        {
                        	strcpy(itemName, dir->d_name); //copy the dir file name and folder to itenName array
				strcat(itemNameList, itemName);//concatenate two line
				strcat(itemNameList, "\n"); //putting line break
			}
                }
        }
    	
 	writen(socket, itemNameList, strlen(itemNameList)); //writing into socket
        closedir(d); //closing dir

	serverLog = fopen(serverLogName, "a");//open in append mode so it doesn't previously stored content
	fprintf(serverLog, "called dir, list of file names in directory sent to client.\n");
	fclose(serverLog);
}





void processCd(int socket, FILE* serverLog)//writing to server log does not function properly
{

	char param[MAX_BLOCK_SIZE]={0};
	char msgToClient[MAX_BLOCK_SIZE]={0};
	int paramFlag = 0;

	readn(socket,param,sizeof(param)); //read for parameter if possible because client will send parameter regardless

	if(strcmp(param, "#")!=0)//if there is a parameter set paramflag to 1
	{
		paramFlag = 1;
	}

	
	if(paramFlag>0)//if there is a paramter ch
	{
		if(strcmp(param, ".")==0)//normally chdir would not change directory with parameter '.' so to differentiate this, a special message is sent is returned to the client.
		{
			getcwd(msgToClient, sizeof(msgToClient));
			strcat(msgToClient, " is the current directory because parameter '.' means no change.");
		}
		else if(strcmp(param, "~")==0)
		{
			chdir(getenv("HOME")); //moving into home directory
			getcwd(msgToClient, sizeof(msgToClient));//getting current working directory 
			strcat(msgToClient, " is the current directory.\n");
		}
		else
		{
			if(chdir(param)<0)//upon finding no such directory chdir returns a value lower than 0
			{
				strcat(msgToClient,"No such directory found.\n");
			}
			else//because ".." "/" "~" and many more parameters of cd is innately supported by chdir.
			{
				getcwd(msgToClient, sizeof(msgToClient));
				strcat(msgToClient, " is the current directory.\n");			
			}
		}
	}
	else //because on linux cd without parameter brings you back to home directory
	{
		chdir(getenv("HOME")); //moving into home directory
		getcwd(msgToClient, sizeof(msgToClient)); //getting current active directory
		strcat(msgToClient, " is the current directory.\n");
	}

	writen(socket, msgToClient, strlen(msgToClient)); // writing into socket

	bzero(msgToClient, sizeof(msgToClient)); //clear the message
	bzero(param, sizeof(param));//clear the parameter

	
}
 


void processPut(int socket, FILE* serverLog) //function put
{
    	char fileName[MAX_BLOCK_SIZE]={0};
	int nameClash = 0;
    	readn(socket,fileName, sizeof(fileName));//read file name

	char location[MAX_BLOCK_SIZE]={0};
    	getcwd(location,sizeof(location));//getting the active directory location

        DIR *d; //directory pointer used to open directory path
        struct dirent *dir;
        d = opendir(location); //opening directory path

        if(d)//if directory is successful
        {
                while((dir = readdir(d)) !=NULL) //condition to check whether the directory has files and folder
                {
			if(strcmp(dir->d_name,".")!=0&&strcmp(dir->d_name,"..")!=0)//'.' and ".." is not supposed to be a folder so its removed
                        {
                        	if(strcmp(dir->d_name, fileName)==0)//condition to check whether there is same filename or not
				{
					++nameClash;  //increasing counter + 1 when name clash
				}
			}

                }
        }

	closedir(d); //closing dir

	if(nameClash>0)//write opcode to client to denote file name clash
	{
		bzero(fileName,sizeof(fileName));
		writen(socket, fileNameClash, strlen(fileNameClash));
		serverLog = fopen(serverLogName, "a");//open in append mode so it doesn't previously stored content
		fprintf(serverLog, "Put failure : Client tried to put a file with clashing name.\n");
		fclose(serverLog);
	}
	else if(nameClash==0)//write opcode to client to denote file name is ok and create file ready to receive data from client
	{
		writen(socket, fileNameOkay, strlen(fileNameOkay)); //writing socket

		char writeFile[MAX_BLOCK_SIZE];
		FILE *file; //file pointer for file descriptor
    		file = fopen(fileName,"w"); //opening file in writing mode
    		bzero(fileName,sizeof(fileName)); //clear the filename before writing to file
		
		readn(socket,writeFile,sizeof(writeFile)); //reading data from socket
		while(strcmp(writeFile,EOF_Message) != 0) //checking condition to set when there is eof message 
		{
			fwrite(writeFile,sizeof(char),strlen(writeFile),file); //writing data into text file
            		bzero(writeFile, sizeof(writeFile));
			readn(socket,writeFile,sizeof(writeFile)); //reading from socket and writing into text
		}
		bzero(writeFile, sizeof(writeFile));
    		fclose(file); //closing file
    		serverLog = fopen(serverLogName, "a");//open in append mode so it doesn't previously stored content
		fprintf(serverLog, "Put success : Client successfully placed file.\n");
		fclose(serverLog); //closing server log
	}
	else//any other error occured
	{
		bzero(fileName,sizeof(fileName));
		writen(socket, miscError, strlen(miscError)); //writing into socket
		serverLog = fopen(serverLogName, "a");//open in append mode so it doesn't previously stored content
		fprintf(serverLog, "Put error : miscellaneous error.\n");
		fclose(serverLog);
	}
}






void processGet(int socket, FILE* serverLog) //function get
{
	char fileName[MAX_BLOCK_SIZE]={0};
	char location[MAX_BLOCK_SIZE]={0};
	int fileFound=0;

	readn(socket,fileName, sizeof(fileName));//read file name
    	getcwd(location,sizeof(location));

        DIR *d; //dir pointer for directory descriptor
        struct dirent *dir;
        d = opendir(location); //opening directory location

        if(d) //if directory is successful
        {
                while((dir = readdir(d)) !=NULL) //while there is filename path
                {
			if(strcmp(dir->d_name,".")!=0&&strcmp(dir->d_name,"..")!=0)//'.' and ".." is not supposed to be a folder so its removed
                        {
                        	if(strcmp(dir->d_name, fileName)==0)
				{
					++fileFound; // + 1 incrementation if same file name found
				}
			}

                }
        }
	closedir(d);//close dir
	bzero(location,sizeof(location));

	if(fileFound>0)
	{
		//let client know file is available on the server and open(read) file and write to client.
		writen(socket, fileAvailable, strlen(fileAvailable));//send code to let client know file is available
		char readFile[1000]={0};
		FILE *file;//file pointer for file descriptor
    		file = fopen(fileName,"r");//open file stream in read mode
    		bzero(fileName,sizeof(fileName)); //clear the filename after opening the file
		if(file == NULL)//Error opening file so server sends error code to client
		{
			writen(socket, fileOpenError, sizeof(fileOpenError));//send error code/signal to client
		}
		else//open and read file and write into socket.
		{
			while((fread(readFile,sizeof(char),sizeof(readFile),file))>0)//read from file
			{
				writen(socket,readFile,strlen(readFile));
				bzero(readFile,sizeof(readFile));
			}
			writen(socket,EOF_Message,sizeof(EOF_Message));//eof message to notify end of file to client			
		}
		fclose(file);//close file stream

		serverLog = fopen(serverLogName, "a");//open in append mode so it doesn't previously stored content
		fprintf(serverLog, "Get success : file data sent to client.\n"); //writing response into server log
		fclose(serverLog);//closing server log
	}
	else
	{
		//let client know file is not unavailable
		writen(socket, fileUnavailable, strlen(fileUnavailable));//send code to let client know file is not available
		serverLog = fopen(serverLogName, "a");//open in append mode so it doesn't previously stored content
		fprintf(serverLog, "Get failure : file name not found.\n"); //writing response into server log
		fclose(serverLog); // closing server log
	}
}




void serve_a_client(int sd, FILE* serverLog)//cannot open here because multiple clients have their own child process serving them.
{
    	int nr, nw;
    	char buf[MAX_BLOCK_SIZE];

   	while (1)
	{
	  
		if ((nr = readn(sd, buf, sizeof(buf))) <= 0)  //reading data from socket
		     return;  //connection broken down 
	 
	   
		buf[nr] = '\0';

		if(strcmp(buf,"pwd")==0) //condition to proceed next step only the command is pwd
	     	{
			bzero(buf, sizeof(buf));
			processpwd(sd, serverLog); //proceeding to processpwd
	     	}
	     	else if(strcmp(buf,"dir")==0)//condition to proceed next step only the command is dir
	     	{
			bzero(buf, sizeof(buf));
			processdir(sd, serverLog); //proceeding to processdir
	     	}
	     	else if(strcmp(buf,"cd")==0)//condition to proceed next step only the command is cd
	     	{
			bzero(buf, sizeof(buf));
			processCd(sd, serverLog); //proceeding to cd function
	     	}
	     	else if(strcmp(buf,"put")==0)//condition to proceed next step only the command is put
	     	{
			bzero(buf, sizeof(buf));
			processPut(sd, serverLog); //proceeding to put function
	     	}
		else if(strcmp(buf, "get")==0)//condition to proceed next step only the command is get
		{
			bzero(buf, sizeof(buf));
			processGet(sd, serverLog); //proceeding to get function
		}
	}

}
 
 
     
 
int main(int argc, char *argv[])//must write server log here because once if multiple clients try to open the log it will clash.
{
     int sd, nsd, n;  
     pid_t pid;
     unsigned short port;   // server listening port
     socklen_t cli_addrlen;  
     struct sockaddr_in ser_addr, cli_addr; 
      
     /* get the port number */
     if (argc == 1) {
          port = SERV_TCP_PORT;
     } else if (argc == 2) {
          int n = atoi(argv[1]);   
          if (n >= 1024 && n < 65536) 
              port = n;
          else {
              printf("Error: port number must be between 1024 and 65535\n");
              exit(1);
          }
     } else {
          printf("Usage: %s [ server listening port ]\n", argv[0]);     
          exit(1);
     }
 
     /* turn the program into a daemon */
     daemon_init(); 
 
     /* set up listening socket sd */
     if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
           perror("server:socket"); exit(1);
     }
     
 
     /* build server Internet socket address */
     bzero((char *)&ser_addr, sizeof(ser_addr));
     ser_addr.sin_family = AF_INET;
     ser_addr.sin_port = htons(port);
     ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

     /* note: accept client request sent to any one of the
        network interface(s) on this host. 
     */
 
     /* bind server address to socket sd */
     if (bind(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0){
           perror("server bind"); exit(1);
     }
  
 
     /* become a listening socket */
     listen(sd, 5);
     printf("Listening to the client connection\n");

     FILE* serverLog;//file descriptor for server log
     serverLog = fopen(serverLogName, "w");//open in write mode so it removes previously stored content(reset log)
     fprintf(serverLog, "%s", newLogMessage);
     fclose(serverLog);
 
     while (1) {
 
          /* wait to accept a client request for connection */
          cli_addrlen = sizeof(cli_addr);
	  printf("Accepting client address connection\n");
          nsd = accept(sd, (struct sockaddr *) &cli_addr, &cli_addrlen);
     
          if (nsd < 0) {
               if (errno == EINTR)   /* if interrupted by SIGCHLD */
                    continue;
               perror("server:accept"); exit(1); //error message
          }
 
          /* create a child process to handle this client */
          if ((pid=fork()) <0) {
              perror("fork"); exit(1);
          } else if (pid > 0) { 
              close(nsd);
              continue; /* parent to wait for next client */
          }
 
          /* now in child, serve the current client */
          close(sd); 
          serve_a_client(nsd, serverLog);//parse file descriptor as another parameter.
          exit(0);
     }
}

