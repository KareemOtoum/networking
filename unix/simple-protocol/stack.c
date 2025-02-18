#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>

#define PORT "8080"

int main()
{
    struct addrinfo hints, *serverinfo;
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    int rv;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // dont care about ipv4 or ipv6
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    if((rv = getaddrinfo("127.0.0.1", PORT, &hints, &serverinfo) != 0))
    {
        fprintf(stderr, "gettaddrinfo error\n");
        return 1;
    }

    int serverSock = socket(serverinfo->ai_family, serverinfo->ai_socktype,
       serverinfo->ai_protocol);

    if(serverSock == -1)
    {
        perror("Server: socket\n"); 
        exit(1);    
    } 

    if(bind(serverSock, serverinfo->ai_addr, serverinfo->ai_addrlen) == -1)
    {
        close(serverSock);
        perror("Server: bind\n");
        exit(1);
    }

    printf("Binded to port: %s\n", PORT);

    freeaddrinfo(serverinfo);

    if(listen(serverSock, 1) == -1)
    {
        perror("Server: listen\n");
        exit(1);
    }

    printf("Server: waiting for connections...\n");

    serverSock = accept(serverSock, (struct sockaddr *)&client_addr, &sin_size);
    if(serverSock == -1)
    {
        perror("Server: accept");
        exit(1);
    }
    
    char ip4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (struct sockaddr_in *)&client_addr, ip4, INET_ADDRSTRLEN);
    printf("Got connection from: %s!\n", ip4);

    char response[] = "HTTP/1.1 200 OK\n"
    "Content-Type: text/html\n"
    "Content-Length: 21\n"
    "\n"
    "<h1>Hello World!</h1>";


    if(send(serverSock, response, sizeof(response), 0) == -1)
    {
        perror("Server: send");
        exit(1);
    }
    printf("Sent Hello World! closing down...\n");
    
    sleep(5);

    close(serverSock);
    return 0;
}