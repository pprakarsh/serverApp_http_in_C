#include "httpd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>

// Maximum number of client connections has been set to 1000
#define MAX_CONNECTIONS 1000

static void openConnection(const char *);		//starts the server
static void clientResponse(int);			//takes care of response to particular client which is requesting for a service from the server
static int serverFileDescriptor;
int * clients;						//Array containing client socket file descriptors
static int clientFileDescriptor;
static char *buffer;					//contains the received request from the client

void server(const char *PORT) {
  struct sockaddr_in clientAddress;
  socklen_t addressLength;

  printf("Address of server is http://127.0.0.1:%s\n",PORT);
  
  clients = malloc(sizeof(int)*1000);
 
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++)
    clients[i] = -1;
  openConnection(PORT);

 int main_socket = serverFileDescriptor; int new_socket; int max_clients = MAX_CONNECTIONS; int maxSocketDescriptor; int socketDescriptor; int activity; fd_set readfds; 

  while (1) {
    addressLength = sizeof(clientAddress);
  
    FD_ZERO(&readfds);

    FD_SET(main_socket, &readfds);
    maxSocketDescriptor = main_socket;

    for ( i = 0 ; i < max_clients ; i++)
    {
       socketDescriptor = clients[i];

       if(socketDescriptor > 0)
          FD_SET( socketDescriptor , &readfds);

       if(socketDescriptor > maxSocketDescriptor)
           maxSocketDescriptor = socketDescriptor;
     }
     
     activity = select( maxSocketDescriptor + 1 , &readfds , NULL , NULL , NULL);

     if ((activity < 0))
     {
        printf("select error");
     }

     if (FD_ISSET(main_socket, &readfds))
     {
        if ((new_socket = accept(main_socket, (struct sockaddr *)&clientAddress, (socklen_t*)&addressLength))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
         }     
	 
         int client_slot = 2000; 
         for (i = 0; i < max_clients; i++)   
         {     
            if( clients[i] == -1 )   
            {   
               clients[i] = new_socket;   
	       client_slot = i;
               printf("Adding to list of sockets as %d\n" , i);   
               break;   
             }   
           } 
	   clientResponse(client_slot);
	}
  }
}

void openConnection(const char *port) {
  struct addrinfo hostAddress, *hostAddress1, *iterator;

  memset(&hostAddress, 0, sizeof(hostAddress));
  hostAddress.ai_family = AF_INET;
  hostAddress.ai_socktype = SOCK_STREAM;
  hostAddress.ai_flags = AI_PASSIVE;				
  
  if (getaddrinfo(NULL, port, &hostAddress, &hostAddress1) != 0) {
    perror("getaddrinfo() error");
    exit(1);
  }

  for (iterator = hostAddress1; iterator != NULL; iterator = iterator->ai_next) {
    int option = 1;
    serverFileDescriptor = socket(iterator->ai_family, iterator->ai_socktype, 0);
    setsockopt(serverFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (serverFileDescriptor == -1)
      continue;
    if (bind(serverFileDescriptor, iterator->ai_addr, iterator->ai_addrlen) == 0)
      break;
  }
  if (iterator == NULL) {
    perror("socket() or bind()");
    exit(1);
  }

  freeaddrinfo(hostAddress1);			

  if (listen(serverFileDescriptor, 1000000) != 0) {
    perror("listen() error");
    exit(1);
  }
}

void clientResponse(int n) 
{
  int receivedLength;
  FILE* clfp = fopen("client.txt", "w");

  buffer = malloc(65535);
  receivedLength = recv(clients[n], buffer, 65535, 0);

  if (receivedLength < 0) 
    fprintf(stderr, ("recv() error\n"));
  else if (receivedLength == 0) 
    fprintf(stderr, "Client disconnected upexpectedly.\n");
  else
  {
    buffer[receivedLength] = '\0';

    requestMethod = strtok(buffer, " \t\r\n");
    uri = strtok(NULL, " \t");		
    protocol = strtok(NULL, " \t\r\n");	

    fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", requestMethod, uri);

    qs = strchr(uri, '?');

    header_struct *header = requestheader;
    char *t, *t2;
    while (header < requestheader + 16) {
      char *key, *value, *t;

      key = strtok(NULL, "\r\n: \t");
      if (!key)
        break;

      value = strtok(NULL, "\r\n");
      while (*value && *value == ' ')
        value++;

      header->name = key;
      header->value = value;
      header++;
      fprintf(stderr, "[H] %s: %s\n", key, value);
      t = value + 1 + strlen(value);
      if (t[1] == '\r' && t[2] == '\n')			
        break;
    }

    clientFileDescriptor = clients[n];
    dup2(clientFileDescriptor, fileno(clfp));
    close(clientFileDescriptor);

    route(clfp);

    fflush(clfp);
    shutdown(fileno(clfp), SHUT_WR);
    fclose(clfp);
  }

  shutdown(
      clientFileDescriptor,
      SHUT_RDWR);
  close(clientFileDescriptor);
  clients[n] = -1;
}

