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

#define MAX_CONNECTIONS 1000

static int listenfd;
int * clients;
static void error(char *);
static void startServer(const char *);
static void respond(int);

static int clientfd;

static char *buf;

void serve_forever(const char *PORT) {
  struct sockaddr_in clientaddr;
  socklen_t addrlen;
  char c;

  int slot = 0;

  printf("Server started %shttp://127.0.0.1:%s%s\n", "\033[92m", PORT,
         "\033[0m");
  
  // create shared memory for client slot array
	clients = malloc(sizeof(int)*1000);
  // Setting all elements to -1: signifies there is no client connected
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++)
    clients[i] = -1;
  startServer(PORT);

  // Ignore SIGCHLD to avoid zombie threads
  signal(SIGCHLD, SIG_IGN);


  int master_socket = listenfd; int new_socket; int max_clients = MAX_CONNECTIONS; int max_sd; int sd; int activity; fd_set readfds; 

  // ACCEPT connections
  while (1) {
	  printf("I am new iteration\n");
    addrlen = sizeof(clientaddr);
    //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = clients[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&clientaddr, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }   
		//add new socket to array of sockets  
		int client_slot = 2000; 
            for (i = 0; i < max_clients; i++)   
            {   
                //if position is empty  
                if( clients[i] == -1 )   
                {   
                    clients[i] = new_socket;   
		    client_slot = i;
                    printf("Adding to list of sockets as %d\n" , i);   
                    break;   
                }   
            } 
	    respond(client_slot);
	    printf("Responded to client\n");
	}
  }

}

// start server
void startServer(const char *port) {
  struct addrinfo hints, *res, *p;

  // getaddrinfo for host
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;				//for binding sockets this flag is set
  if (getaddrinfo(NULL, port, &hints, &res) != 0) {
    perror("getaddrinfo() error");
    exit(1);
  }

  // socket and bind
  for (p = res; p != NULL; p = p->ai_next) {
    int option = 1;
    listenfd = socket(p->ai_family, p->ai_socktype, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (listenfd == -1)
      continue;
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
      break;
  }
  if (p == NULL) {
    perror("socket() or bind()");
    exit(1);
  }

  freeaddrinfo(res);			//frees allocated linked list memory to res

  // listen for incoming connections
  if (listen(listenfd, 1000000) != 0) {
    perror("listen() error");
    exit(1);
  }
}

// get request header by name
char *request_header(const char *name) {
  header_t *h = reqhdr;
  while (h->name) {
    if (strcmp(h->name, name) == 0)
      return h->value;
    h++;
  }
  return NULL;
}

// get all request headers
header_t *request_headers(void) {
  return reqhdr;
}

// client connection
void respond(int n) 
{
	printf("Value of n: %d\n", n);
  int rcvd, fd, bytes_read;
  char *ptr;
  FILE* clfp = fopen("client.txt", "w");

  buf = malloc(65535);
  rcvd = recv(clients[n], buf, 65535, 0);

  if (rcvd < 0) // receive error
    fprintf(stderr, ("recv() error\n"));
  else if (rcvd == 0) // receive socket closed
    fprintf(stderr, "Client disconnected upexpectedly.\n");
  else // message received
  {
    buf[rcvd] = '\0';

    method = strtok(buf, " \t\r\n");
    uri = strtok(NULL, " \t");		//The page/subpage to be visited
    prot = strtok(NULL, " \t\r\n");	//http here

    fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);

    qs = strchr(uri, '?');	//First occurence of a character

    header_t *h = reqhdr;
    char *t, *t2;
    while (h < reqhdr + 16) {
      char *k, *v, *t;

      k = strtok(NULL, "\r\n: \t");
      if (!k)
        break;

      v = strtok(NULL, "\r\n");
      while (*v && *v == ' ')
        v++;

      h->name = k;
      h->value = v;
      h++;
      fprintf(stderr, "[H] %s: %s\n", k, v);
      t = v + 1 + strlen(v);
      if (t[1] == '\r' && t[2] == '\n')			//Signal completion of header
        break;
    }

    // bind clientfd to stdout, making it easier to write
    clientfd = clients[n];
  printf("route starts!\n");
    dup2(clientfd, fileno(clfp));
    close(clientfd);

    // call router
    route(clfp);

    // tidy up
    fflush(clfp);
    shutdown(fileno(clfp), SHUT_WR);
    fclose(clfp);
  }

  // Closing SOCKET
  shutdown(
      clientfd,
      SHUT_RDWR); // All further send and recieve operations are DISABLED...
  close(clientfd);
  clients[n] = -1;
}

