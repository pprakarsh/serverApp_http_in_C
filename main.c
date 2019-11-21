#include "httpd.h"
#include<sys/socket.h>
#include<sys/types.h>

int main(int c, char **v) {
  serve_forever("8000");
  return 0;
}

void route(FILE* clfp)
{
	printf("I am in route\n");
	if((strcmp(uri, "/") == 0) && (strcmp(method, "GET") == 0))
	{
	    fprintf(clfp, "HTTP/1.1 200 OK\r\n\r\nHello! You are using %s", request_header("User-Agent"));
	}
	else if((strcmp(uri, "/test") == 0) && (strcmp(method, "GET") == 0))
	{
	    fprintf(clfp, "HTTP/1.1 200 OK\r\n\r\n");
	    fprintf(clfp, "List of request headers:\r\n\r\n");
	    header_t *h = request_headers();

	    while (h->name) 
	    {
	      fprintf(clfp, "%s: %s\n", h->name, h->value);
	      h++;
	    }
	}
	else
	{
		fprintf(clfp, "HTTP/1.1 200 OK\r\n\r\n");
		fprintf(clfp, "HTTP/1.1 500 Internal Server Error\n\nThe server has no handler to the request.\n");
	}
}
