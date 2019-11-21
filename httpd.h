#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <string.h>
#include <stdio.h>

// Server control functions

void serve_forever(const char *PORT);

// Client request

char *method, // "GET" or "POST"
    *uri,     // "/index.html" things before '?'
    *qs,      // "a=1&b=2"     things after  '?'
    *prot;    // "HTTP/1.1"

char *payload; // for POST
int payload_size;

char *request_header(const char *name);

typedef struct { char *name, *value; } header_t;
static header_t reqhdr[17] = {{"\0", "\0"}};
header_t *request_headers(void);

// user shall implement this function

void route(FILE* fp);

#endif
