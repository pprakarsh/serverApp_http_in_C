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
#include<pthread.h>

//Maximum number of clients is restricted to 1000
#define MAX_CONNECTIONS 1000

static void openConnection(const char *);		//starts the server
static void clientResponse(int, int);			//takes care of response to particular client which is requesting for a service from the server
void* connection_handler(void *socket_desc) ;		//Individual threads mapped to individual thread run this function

static int serverFileDescriptor;
int * clients;						//Array containing client socket file descriptors
static int clientFileDescriptor;
static char *buffer;					//contains the received request from the client

struct arg
{
	int slot;
	int client_no;
};

void swap(char* c1, char* c2)
{
	char* temp = c1;
	c1 = c2;
	c2 = temp;
}

void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        swap((str+start), (str+end));
        start++;
        end--;
    }
}

char* itoa(int num, char* str, int base)
{
    int i = 0;
    int isNegative = 0;

    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (num < 0 && base == 10)
    {
        isNegative = 1;
        num = -num;
    }

    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; 

    reverse(str, i);

    return str;
}


void server(const char *PORT) {
  struct sockaddr_in clientAddress;
  socklen_t addressLength;
  char c;

  int slot = 0;

  printf("Server started %shttp://127.0.0.1:%s%s\n", "\033[92m", PORT,
         "\033[0m");
  
  clients = malloc(sizeof(int)*1000);
  
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++)
    clients[i] = -1;

  openConnection(PORT);

  signal(SIGCHLD, SIG_IGN);

int counter = 0;
  while (1) {
    ++counter;
    addressLength = sizeof(clientAddress);
    clients[slot] = accept(serverFileDescriptor, (struct sockaddr *)&clientAddress, &addressLength);
    pthread_t thread_id;
    struct arg* argv = malloc(sizeof(struct arg));
    argv->slot = slot;
    argv->client_no = counter;
    if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) argv) < 0)
        {
            perror("could not create thread");
        }
    while (clients[slot] != -1)
      slot = (slot + 1) % MAX_CONNECTIONS;
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

void *connection_handler(void *socket_desc) 
{
	pthread_detach(pthread_self());
	struct arg* argv = (struct arg* )socket_desc;	
	int z = argv->slot;
	int client_no = argv->client_no;
	clientResponse(z, client_no);
	pthread_exit(0);	
}

void clientResponse(int n, int client_no) 
{
  int clientFileDescriptor;
  char *buffer;
  char *method, 
    *uri,     
    *qs,      
    *prot,    
    *payload; 

  int payload_size;

  int receivedLength;
  char fname[] = "client";
  char index[10];
  itoa(client_no, index, 10);

  char ext[] = ".txt";
  strcat(index, ext);
  strcat(fname, index);
  FILE* clfp = fopen(fname, "w");


  buffer= malloc(65535);
  receivedLength = recv(clients[n], buffer, 65535, 0);

  if (receivedLength < 0)
    fprintf(stderr, ("recv() error\n"));
  else if (receivedLength == 0) 
    fprintf(stderr, "Client disconnected upexpectedly.\n");
  else 
  {
    buffer[receivedLength] = '\0';

    method = strtok(buffer, " \t\r\n");
    uri = strtok(NULL, " \t");		
    prot = strtok(NULL, " \t\r\n");	

    fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);

    qs = strchr(uri, '?');	

    header_t *header = requestheader;
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

    route(clfp, uri, method);

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

