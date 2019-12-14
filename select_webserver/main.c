#include "httpd.h"
#include<sys/socket.h>
#include<sys/types.h>
#include<stdlib.h>

int main(int c, char **v) {
  server("8000");
  return 0;
}

char* readfrom_HTML_file(char* filename)
{
	char c;
	FILE* fptr;

	char* str = malloc(sizeof(char)*1000000);
	str[0] = '\0';

	fptr = fopen(filename, "r");
	if (fptr == NULL)
	{
		printf("Cannot open file %s\n", filename);
		exit(0);
	}
	// Read contents from file
	c = fgetc(fptr);
	while (c != EOF)
	{
		strncat(str, &c, 1);
		c = fgetc(fptr);
	}
	fclose(fptr);
	return str;
}

void route(FILE* clfp)
{
	if((strcmp(uri, "/") == 0) && (strcmp(requestMethod, "GET") == 0))
	{
	    char filename[] = "./homepage.html";
	    char* str = readfrom_HTML_file(filename);
	    char resp[] = "HTTP/1.1 200 Ok\r\n"
		          "Content-Type:text/html\r\n\r\n";
	    fprintf(clfp, "%s", resp);
	    fprintf(clfp, "%s", str);
	}
	else if((strcmp(uri, "/prakarsh") == 0) && (strcmp(requestMethod, "GET") == 0))
	{
	    char filename[] = "./prakarsh.html";
	    char* str = readfrom_HTML_file(filename);
	    fprintf(clfp, "HTTP/1.1 200 OK\r\n\r\n");
	    fprintf(clfp, "%s", str);
	}
	else if((strcmp(uri, "/vibhor") == 0) && (strcmp(requestMethod, "GET") == 0))
	{
	    char filename[] = "./vibhor.html";
	    char* str = readfrom_HTML_file(filename);
	    fprintf(clfp, "HTTP/1.1 200 OK\r\n\r\n");
	    fprintf(clfp, "%s", str);
	}
	else
	{
		fprintf(clfp, "HTTP/1.1 200 OK\r\n\r\n");
		fprintf(clfp, "HTTP/1.1 500 Internal Server Error\n\nThe server has no handler to the request.\n");
	}
}
