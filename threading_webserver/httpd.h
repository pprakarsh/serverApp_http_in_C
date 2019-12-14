#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <string.h>
#include <stdio.h>

// function to start server

void server(const char *PORT);

// components of header

char *method, // request method
    *uri,     // uniform resource identifier
    *qs,      // "a=1&b=2"     things after  '?'
    *prot;    // protocol

typedef struct { char *name, *value; } header_t;
static header_t requestheader[17] = {{"\0", "\0"}};

void route(FILE* , char* , char* );

#endif
