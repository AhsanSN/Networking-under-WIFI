/**
Note:
This code is almost same as the code that was provided to us in lab 7.
The reason is, all the work is being done by the server.
**/

//imports
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <pthread.h>
#include <stdbool.h>


//function declared below
void error(char *msg);//returns error
void* get_msg(void* fd);//reads from buffer
void* send_msg(void* fd);//write to buffer

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);//char to int
    sockfd = socket(AF_INET, SOCK_STREAM, 0);//creating a socket
    if (sockfd < 0) 
        error("ERROR opening socket");

    ///setting server adress attributes
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    in_addr_t t = inet_addr(argv[1]);
    bcopy((char *) &t, 
         (char *)&serv_addr.sin_addr.s_addr,
         sizeof(in_addr_t));

    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) //establishing connection
        error("ERROR connecting");    
      
    //sending client name to server  
    if ((n = write(sockfd,argv[3],strlen(argv[3])))<0)
    {
    	error("ERROR writing to socket");
    }        
    bzero(buffer,256);
    //reading from server
    n = read(sockfd,buffer,255);
        if (n < 0) 
            error("ERROR reading from socket");
    if (strncmp(buffer,"denied",strlen(buffer))==0)
    {
        printf("ERROR Couldn't connect\n");
        exit(0);
    }
    pthread_t rthrptr;
    pthread_t wthrptr;
    /**
    Here we create to threads.
    Both thread will run different function.
    One will read from server
    Other will write to server
    **/
    pthread_create(&rthrptr, NULL,get_msg,&sockfd);
    pthread_create(&wthrptr, NULL,send_msg,&sockfd);
    /**
    threads have been created.
    But the client process must not stop.
    **/
    while(true); 

    //just in case
    return 0;
}


void error(char *msg)
{
    perror(msg);
    exit(0);
}

void* get_msg(void* fd)
{
    int sockfd = *((int*)fd);
    int n;
    char buffer[256];
    //for ever loop
    //runs while '/quit' isnt sent
    while(true)
    {
        printf(">> ");
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        //writting to buffer
        if ((write(sockfd,buffer,strlen(buffer)))<0)
        {
        	error("ERROR writing to socket");
        }            
        /**
        here the quit command disconnects the client.
        The server detects it, and disconnects it from there too.
        **/
        if(strncmp(buffer,"/quit",5)==0)
        {
            exit(0);
        }
    }
}

void* send_msg(void* fd)
{
	int n;
    int sockfd =*((int*) fd);
    //for ever loop
    //runs while '/quit' isnt sent
    while(true)
    {
       bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) 
            error("ERROR reading from socket");
        printf("%s\n",buffer);
    }
}
