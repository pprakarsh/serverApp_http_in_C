#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <string.h>
#include <stdio.h>

//Server function prototype to start server	

void server(const char *PORT);

// Important info of Request from client

char *requestMethod, // Request Method is to be specified here
    *uri,     // Uniform Resource Indicator of the resource
    *qs,      // "a=1&b=2"     things after  '?'
    *protocol;    // "HTTP/1.1"

typedef struct { char *name, *value; } header_struct;
static header_struct requestheader[17] = {{"\0", "\0"}};

// Function to respond to client request

void route(FILE* filePointer);

#endif
