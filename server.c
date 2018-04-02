//imports
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

typedef struct{
char Name[100]; //stores name of the client
int client_id; //stores socket
bool connected; //whether the client is connected or not
pthread_t* thread_ptr; //unique thread for each client   
} client; //a struct that stores all the information of a client

/**
this program run for only 10 clients
**/
int n_clients =0;
pthread_t* thread[10];//unique thread for each client   
int allowed_clients = 10;
client client_ary[10];//array of clients

//function defined below main
void error(char *msg);//returns error
void showClients(char *buffer, int i);//show clients
int quitClient(int i);
void sendMsg(char *buffer, int i);
void* recv_input(void* index);///thread function

int main(int argc, char *argv[])
{
/**
Code written below has been taken from the lab activity that we did.
Therefore it will look similiar.
**/
    int sockfd, newsockfd; 
    int portno; 
    int clilen; 
    char buffer[256];
    int n;

//gives a seperate thread to each client
int n_thread=0;//count of threads

struct sockaddr_in serv_addr, cli_addr; 

if (argc < 2) {//if we have not received port number
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
}

sockfd = socket(AF_INET, SOCK_STREAM, 0);//creating socket

if (sockfd < 0) 
    error("ERROR opening socket");

//setting buffer to 0. To erase all previous values
bzero((char *) &serv_addr, sizeof(serv_addr));

// char to int
portno = atoi(argv[1]);

serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = INADDR_ANY;
serv_addr.sin_port = htons(portno);

if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)//binding Socket to address 
    error("ERROR on binding");

listen(sockfd,5); //listening to the socket for client
clilen = sizeof(cli_addr);

/**
Here comes the real part. Server will run a while loop to accept request from clients.
**/     
while(true)
{
    if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))<0)
    {
        error("ERROR on accept");
} //accepts message from client
bool validated = true;
int index = -1;

if ((n = read(newsockfd,buffer,255))<0)
{
    error("ERROR reading from socket");
}                
for (int i=0;i<allowed_clients;i++)
{
    if(client_ary[i].connected && strncmp(client_ary[i].Name,buffer,strlen(buffer))==0)
    {
        validated = false;
        if ((n = write(newsockfd,"denied",18))<0)
        {
            error("ERROR writing to socket");
        }                        
        break;
    }
    else if(client_ary[i].connected==false)
    {
        index =i;
    }  
}
//client connected
if ((validated==true) && (index>-1))
{
n = write(newsockfd,"Connected",18);//client connected
client_ary[index].client_id = newsockfd;
strncpy(client_ary[index].Name,buffer,strlen(buffer));//saving client's name
printf("%s connected.\n",client_ary[index].Name);
n_clients++;
client_ary[index].connected = true;//the ith client has been connected
/**
Here we will create a new thread.
This thread will now be taking inputs and managing them
each client will have a seperate thread.
**/
thread[n_thread] = malloc(sizeof(pthread_t));
client_ary[index].thread_ptr = thread[n_thread];
pthread_create(thread[n_thread], NULL,recv_input, &index);//creating thread
n_thread++;
}
}
return 0;
}


void error(char *msg)// error printing function
{
    perror(msg);
    exit(1);
}

void* recv_input(void* index)
{
    int n;
    char buffer[256];    
    int i = *((int*) index);
    while(true)
    {
bzero(buffer,256);/// setting reading buffer values to zero
if ((n = read(client_ary[i].client_id,buffer,255))<0)/// receiving input
{
    error("ERROR reading from socket");
}            
else
{
//analyzing input
/**
We will compare the input with different commands to see
what is the input
**/
    if(strncmp(buffer,"/list",5)==0)
    {
        showClients(buffer,i);
    }
    else if(strncmp(buffer,"/quit",5)==0)
    {
        quitClient(i);
    }
    else if(strncmp(buffer,"/msg",4)==0)
    {
        sendMsg(buffer,i);
    }
    else{
    //none of the above commands
        if ((n = write(client_ary[i].client_id,"Invalid command",18))<0)
        {
            error("ERROR writing to socket");
        }                        
    }
}   
} 
}

void showClients(char *buffer, int i)//show clients
{
bzero(buffer,256);//setting buffer to zero
strcat(buffer,"Clients: ");
for (int i=0;i<allowed_clients;i++)
{
        if(client_ary[i].connected == true) //if a client is connected
        {
            strcat(buffer,client_ary[i].Name);
            strcat(buffer," \n");
        }
    }
    printf("%d",client_ary[i].client_id);
    n = write(client_ary[i].client_id,buffer,strlen(buffer));
}

int quitClient(int i)
{
    close(client_ary[i].client_id);
    client_ary[i].connected =false;
    pthread_exit(NULL);
    return (0);
}

void sendMsg(char *buffer, int i)
{
    int i;
    char str[100];
    char message[100];
    char receiver[100];
    memset(message,'\0',sizeof(message));
    memset(str,'\0',sizeof(str));
    //getting contents of message
    strcpy(str,buffer);
    char * pch;
    printf ("Splitting string \"%s\" into tokens:\n",str);
    strtok (str," "); //eliminates '/msg'
    strcpy(receiver,strtok (NULL," ")); //stores receiver's name
    pch = strtok (NULL," ");//starts storing message
    while (pch != NULL)
    {
            printf ("%s\n",pch);
        strcat(message,pch);
        strcat(message," ");
            pch = strtok (NULL, " ");
    }
    printf (" receiver: %s\n",receiver);
    printf (" message: %s\n",message);
    //sending message
    /**
    int l=5;
    char str[10]={'0','0','0','0','0','0','0','0','0','\0'};
    int j =0;
    while(buffer[l]!=' ' && j<9)
    {
        str[j] = buffer[l];
        l++;
        j++;
    }
    str[j]='\0';
    char message[256];
    bzero(message,strlen(message));
    int m = 0 ;
    l++;
    while(m<strlen(client_ary[i].Name))
    {
        message[m]=client_ary[i].Name[m];
        m++;
    }

    message[m] ='>';
    m++;
    while(l<strlen(buffer))
    {
        message[m] = buffer[l];
    l++;
        m++;
    }
    for (int k=0;k<allowed_clients;k++)
    {
        if(strncmp(client_ary[k].Name,str,10)==0)
        {
            if ((n = write(client_ary[k].client_id,message,256))<0)
            {
                error("ERROR writing to socket");
            }                            
            break;
        }
    }
    **/
}