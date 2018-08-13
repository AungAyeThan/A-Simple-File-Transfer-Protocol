/*
 *  Filename : myftp.c
 * 	Purpose	 : client program
 *	Author 	 : Aung Aye Than
 *  Date  	 : 25/11/2017
 *  Version  : 1.0
 *	 	. 
 */
#include  <unistd.h>
#include  <dirent.h>
#include  <string.h>
#include  <stdlib.h>
#include  <stdio.h>
#include  <sys/types.h>        
#include  <sys/socket.h>
#include  <netinet/in.h>       /* struct sockaddr_in, htons, htonl */
#include  <netdb.h>            /* struct hostent, gethostbyname() */ 
#include  <string.h>
#include  "stream.h"           /* MAX_BLOCK_SIZE, readn(), writen() */
#include  <unistd.h>
#include  <sys/stat.h>

#define SERV_TCP_PORT  40005 /* default server listening port */
 
#define EOF_Message	"30FM5G"//6byte opcode denoting end of file
#define fileNameOkay 	"0"//File currently found, ready to receive parameter.
#define fileNameClash 	"1"//File already exist, file name clashed.
#define miscError 	"2"//Any other error

#define fileAvailable	"0"//File is found 
#define fileUnavailable	"1"//File is not found
#define fileOpenError	"2"//"F1l30P3N3RR0R"//Error opening file

#define cdWithoutParam "#"

int main(int argc, char *argv[])
{
     	int sd, n, nr, nw, i=0;
     	//char buf[MAX_BLOCK_SIZE];
     	char host[60];
     	unsigned short port;
     	struct sockaddr_in ser_addr; struct hostent *hp;
 
     	/* get server host name and port number */
     	if (argc==1) 
     	{  /* assume server running on the local host and on default port */
          	gethostname(host, sizeof(host));
          	port = SERV_TCP_PORT;
     	} 
     	else if (argc == 2) 
     	{ /* use the given host name */
          	strcpy(host, argv[1]);
          	port = SERV_TCP_PORT;
     	} 
     	else if (argc == 3) 
     	{ // use given host and port for server
          	strcpy(host, argv[1]);
          	int n = atoi(argv[2]);
          	if (n >= 1024 && n < 65536)
      		{
              		port = n;
      		}
		else
      		{
              		printf("Error: server port number must be between 1024 and 65535\n");
              		exit(1);
        	}
     	} 
     	else
     	{ 
         	printf("Usage: %s [ <server host name> [ <server listening port> ] ]\n", argv[0]); 
         	exit(1); 
     	}
 
     	/* get host address, & build a server socket address */
     	bzero((char *) &ser_addr, sizeof(ser_addr));
     	ser_addr.sin_family = AF_INET;
     	ser_addr.sin_port = htons(SERV_TCP_PORT);
    	// ser_addr.sin_addr.s_addr = htonl (SERV_INET_NO);
    	if ((hp = gethostbyname(host)) == NULL)
     	{
           	printf("host %s not found\n", host); 
       		exit(1);   
     	}
     	ser_addr.sin_addr.s_addr = * (u_long *) hp->h_addr;
 
     	/* create TCP socket & connect socket to server address */
     	sd = socket(PF_INET, SOCK_STREAM, 0);
     	if (connect(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0) 
     	{ 
          	perror("client connect"); exit(1);
     	}
 
     
    	int size;//variable used to store total length of user input
    	int quitFlag=0;//flag used to terminate input request from client

    	printf("The following commands are available for use :\n");//prompt for client
    	printf("pwd  - Print working directory on server.\n");
    	printf("lpwd - Print working directory on client\n");
    	printf("dir  - List all files within current directory on server.\n");
    	printf("ldir - List all files within current directory on client\n");
    	printf("cd  directory_pathname - Change current directory on server.\n");
    	printf("lcd directory_pathname - Change current directory on client.\n");
    	printf("get file_name - Get text file from server to local machine/client.\n");
    	printf("put file_name - Put text file from local machine/client to server.\n");
    	printf("quit - Terminate current ftp session.\n");

    	while(quitFlag!=1)//while loop to request for input while client has chosen not to quit yet
    	{
		char param[MAX_BLOCK_SIZE]={0}; //must initialise all arrays or else C does not know and some weird characters will be placed inside
		char userInput[MAX_BLOCK_SIZE]={0}; //used to store input that the user has entered
		char cmd[MAX_BLOCK_SIZE]={0}; //used to store the command retrieved from userInput.
		int paramFlag=0; //flag to denote whether a param is available because some functions does not take in paramters
		char buf[MAX_BLOCK_SIZE]={0};//used to store output from the server
		printf("\nmyftp>:"); //prompt for myftp
		fgets(userInput, sizeof(userInput), stdin); //get input from terminal
		size = sizeof(userInput); //store size of userInput into size

		int start=0; //int to store the starting index of userInput
		int end=strlen(userInput)-2;//int end to store the last index of user input -2 because -1 for terminator and another -1 because position starts from 0
	 
		if(end<0)//error check to set end to 0 in case string length is lesser than 2 or else it will cause program to crash
		{
		    	end=0;//
		}
	 
		while(userInput[start]==' ')//removing all spaces from the start of the command like how command line works
		{
		    	++start;//skip blank space
		}
	 
		while(userInput[end]==' ')//removing all spaces from the back of the command like how command line works
		{
		    	end--;//skip blank space
		}
	 
		for(start; start<=end&&paramFlag==0;++start )//for loop for getting cmd
		{
		    	if(userInput[start]==' ')//if statement to set flag to show there is parameter
		    	{
		        	paramFlag=1;//denote there there is a parameter
		    	}
		    	else//to only store character into cmd if it is not a space because if input has cmd and param it will place space into cmd
		    	{
		        	cmd[strlen(cmd)]=userInput[start];//separate and assign command from userInput
		    	}
		}
	 
		while(userInput[start]==' ')// while loop to ignore spaces between cmd and cmdParam
		{
		    	++start;//skip blank space
		}
	 
	 
		for(start; start<=end&&paramFlag==1; ++start)//for loop to retrieve parameter
		{
		    	param[strlen(param)]=userInput[start];//separate and assign parameter from userInput
		}

		if(strcmp(cmd,"quit")==0)//set flag and terminate request for input
		{
		    	quitFlag=1;
		}
		else if(strcmp(cmd,"pwd")==0)//print for working directory from server
		{
			writen(sd,cmd,sizeof(cmd));
			readn(sd,buf,sizeof(buf));
			printf("server reply : %s\n",buf);
			bzero(buf, sizeof(buf));
		}
		else if(strcmp(cmd,"lpwd")==0)//print working directory from local client 
		{
		    	char pathName[MAX_BLOCK_SIZE];
		    	getcwd(pathName, sizeof(pathName));
			printf("%s\n",pathName); //display working directory from client
			bzero(pathName, sizeof(pathName));
		}
		else if(strcmp(cmd,"dir")==0)//list files in current server directory
		{
		    	writen(sd,cmd,sizeof(cmd));
		    	readn (sd,buf,sizeof(buf));
		    	printf("server reply :%s\n",buf);
			bzero(buf, sizeof(buf));
		}
		else if(strcmp(cmd,"cd")==0)//change directory on server
		{
		    	writen(sd,cmd,sizeof(cmd));

			if(paramFlag==0)//write to server op code for a cd command without parameter
			{
				writen(sd, cdWithoutParam, strlen(cdWithoutParam));//send op code to server to denote cd without parameter
			}
			else
			{
		    		writen(sd,param,sizeof(param));//send directory name to server for cd command
			}
		    	readn(sd,buf,sizeof(buf));//read return message from server
		    	printf("%s\n", buf);//display return message
			bzero(buf, sizeof(buf));//clear buffer
		}
		else if(strcmp(cmd, "ldir")==0)//list files from current directory from client
		{
		        char currentLocation[MAX_BLOCK_SIZE]; //array to store current location
		        getcwd(currentLocation, sizeof(currentLocation));//get current working directory and store into current location
		        DIR *d;//dir pointer
		        struct dirent *dir;//dirent pointer
		        d = opendir(currentLocation);//open directory
		        if(d)//if directory is successfully opened
		        {
		            	while((dir=readdir(d))!=NULL)//while there is still file name
		                {
		                      	if(strcmp(dir->d_name,".")!=0&&strcmp(dir->d_name,"..")!=0)//remove . and .. from output as they denote the file directory itself
		                      	{
		               			printf("%s\n",dir->d_name);//display file/folder name
		                      	}
		                }
		        }
		        printf("\n");
		}
	    	else if(strcmp(cmd,"lcd")==0)//change directory on client
		{
			char locName[100];//buffer to store location name

			printf("Current directory %s\n",getcwd(locName,sizeof(locName)));//print current directory before change

			if(paramFlag > 0)//show that there is parameter
			{
				if(chdir(param)!= 0)//if return value is not 0 it will mean unsuccessful
				{
					printf("No such directory\n");//print message to client
				}
			}
			else//go to home directory if there is no parameter
			{
			       	chdir(getenv("HOME"));//change directory to home directory
			}
			printf("Successfully changed directory to : %s\n",getcwd(locName,sizeof(locName)));//print current directory after change
		}
		else if(strcmp(cmd,"put")==0&&paramFlag!=0)//put file from client to server if parameter is stated
		{
		    	char readFile[1000]={0};//create buffer to store data from reading file
			char fileStatus[MAX_BLOCK_SIZE]={0};//1 for op code and 1 for terminator
			char location[MAX_BLOCK_SIZE]={0};//buffer to store current location
			int nameFound=0;//variable to store whether files with similar name is found

		    	getcwd(location,sizeof(location));//get current working directory and store into location

			DIR *d;//dir pointer for current directory
			struct dirent *dir;//dirent pointer to store file/folder information
			d = opendir(location);//open current directory for reading

			if(d)//read file/folder names only if current directory can be opened
			{
				while((dir = readdir(d)) !=NULL)//while there is still available file/folders
				{
					if(strcmp(dir->d_name,".")!=0&&strcmp(dir->d_name,"..")!=0)//'.' and ".." is not supposed to be a folder so its removed
				        {
				        	if(strcmp(dir->d_name, param)==0)//increase namefound counter to show file name is found
						{
							++nameFound;//increment namefound
						}
					}

				}
			}

			closedir(d);//close current directory used for reading.

			if(nameFound>0)//write to server only if file name is found on client
			{
			    	writen(sd,cmd,strlen(cmd));//send command
			    	writen(sd,param,strlen(param));//send file name

				readn(sd, fileStatus, sizeof(fileStatus));//read status before sending

				if(strcmp(fileStatus, fileNameOkay)==0)//only send if file name is okay
				{
					FILE *file;//file pointer for file descriptor.
					file = fopen(param,"r");//open file in read mode, so file will not get overwritten

					if(file == NULL)//if file descriptor is empty
					{
						printf("Error: No file\n");//prompt user of error no such file
					}
				     
					while((fread(readFile,sizeof(char),sizeof(readFile),file))>0)//while file stream still has data
					{
						writen(sd,readFile,strlen(readFile));//write to server data in file stream stored in readFile
						bzero(readFile,sizeof(readFile));//clear buffer for readFile
					}
					writen (sd,EOF_Message,sizeof(EOF_Message));//send eof message to denote end of file
				     
					fclose(file);//close file descriptor
				}
				else if(strcmp(fileStatus, fileNameClash)==0)//print message to let client know file name entered has clashed
				{
					printf("File with name %s already exists, please choose another file.\n", param);//prompt for file name clash
				}
				else//print message for miscellaneous errors
				{
					printf("Error putting file to server\n");//prompt for error putting file onto server
				}
			}
			else//prompt client that file specified is not found on client
			{
				printf("No such file name found, please specify a valid file name.\n");//prompt client no such file is found
			}
		}
		else if(strcmp(cmd, "get")==0&&paramFlag!=0)//get file from server only if parameter is available
		{
			char location[MAX_BLOCK_SIZE]={0};//initialise buffer to store location because c does not ensure a new buffer
			int nameClash=0;//counter for storing clashing names
    			getcwd(location,sizeof(location));//get current working directory and storing into location

        		DIR *d;//dir pointer used to open directory
        		struct dirent *dir;//dirent pointer used to store file/folder information
        		d = opendir(location);//open directory

        		if(d)//read file/folder names only if current directory can be opened
			{
				while((dir = readdir(d)) !=NULL)//while there is still files/folders within the directory
				{
					if(strcmp(dir->d_name,".")!=0&&strcmp(dir->d_name,"..")!=0)//'.' and ".." is not supposed to be a folder so its removed
				        {
				        	if(strcmp(dir->d_name, param)==0)//to increment nameclash counter if file/folder name is found clashing with file name entered
						{
							++nameClash;//increment nameclash counter
						}
					}

				}
			}
			closedir(d);//close directory

			
			if(nameClash==0)//only write to the server the command and parameter if file does not clash
			{
				char fileStatus[MAX_BLOCK_SIZE]={0};//buffer to store acknowledgment code from server, 1 for op code and 1 for terminator
				char writeFile[MAX_BLOCK_SIZE]={0};//array to write into file.
				writen(sd, cmd, strlen(cmd));//write get command to server
				writen(sd, param, strlen(param));//send file name to the server only if the found doesn't clash with an currently existing one at the client.
				readn(sd, fileStatus, sizeof(fileStatus));//read acknowledgment code from server.
			
				if(strcmp(fileStatus, fileAvailable)==0)//read from server only if file
				{
					//create file and read from server
					FILE *file;//file pointer for file descriptor
    					file = fopen(param,"w");//open file in write mode
					readn(sd, writeFile, sizeof(writeFile));//read from socket and store into buffer writeFile to write into file
					if(strcmp(writeFile, fileOpenError)==0)//display message if unable to read file.
					{
						printf("Error reading file %s on server.\n", param);//print error message to let client know file is found on server
					}
					else//read data from socket and write into file if server is able to read file.
					{
						while(strcmp(writeFile,EOF_Message) != 0)//eof message is not received
						{
							fwrite(writeFile,sizeof(char),strlen(writeFile),file);//write into file data found in buffer writeFile
				    			bzero(writeFile, sizeof(writeFile));//clear buffer writeFile because if previous string has a longer string, it will write characters from previous read
							readn(sd,writeFile,sizeof(writeFile));//read from socket and store into writeFile buffer
						}
						bzero(writeFile, sizeof(writeFile));//clear buffer writeFile
					}

					fclose(file);//close file descriptor
				}
				else//do not read because file is not available
				{
					printf("file not available on server, please specify a valid file name \n");//prompt client that file name entered is not available
				}
			}
			else//no need to call from server because file name user entered clashes with one from the client's current directory.
			{
				printf("file name clashed with one currently available, please specify another file name to get.\n");//prompt client that file name has clashed
			}
		}
		else//notify client of invalid command
		{
		    printf("Please enter a valid command. \n");//prompt user to enter a valid command
		}
    	}
    	printf("Client terminating.\n");//notify user of client termination
}
