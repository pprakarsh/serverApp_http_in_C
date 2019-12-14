
This is a very simple HTTP server for handling multiple clients concurrently, using both select and multithreading in C Programming language. It's very easy to use.

## How to use

1. Include header `httpd.h`.
2. Write your route method, handling requests.
3. In main.c `readfrom_HTML_file()` function has been created which can read from any HTML file whose filename is passed as argument to the function. Thus user can create their own HTML pages and pass into the function as argument.
4. Call `serve_forever("8000")` to start serving on http://127.0.0.1:8000/.

See `main.c`, an interesting example.
View `httpd.h` for more information.


## Overview

1. The repository contains 2 directories `select_webserver` and `threading_webserver`.
2. In the `select_webserver` directory the webserver application has been created using select system call.
3. In the `threading_webserver` directory the webserver application has been created using threads.
4. The server listens for incoming requests on the listening port.

    THREADING:
    a. Accept is used to accept any incoming request.
    b. A new thread is created for every request, the thread starts running, at the same time the server application looks          for another request. Thus multiple clients can connect at the same time.
    SELECT:
    a. Select system call is used to check for incoming request.
    b. It returns when it receives an incoming request.The request is served and the loop iterates to select again looking          for another request.
    

## Note
1. Here select does not serve any specific purpose, the application could be implemented without select.
2. But when a server has 2 or more listening ports, select system can be helpful in I/O multiplexing

## Test example

Open http://localhost:8000/ in browser to see request headers.

## Links
http://localhost:8000/prakarsh
http://localhost:8000/vibhor

## Refrences:
Based on <http://blog.abhijeetr.com/2010/04/very-simple-http-server-writen-in-c.html>.
