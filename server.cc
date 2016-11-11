// C++ LIBS
#include <iostream>
#include <bitset>
#include <cmath>
#include <fstream>
#include <thread>

// C LIBS
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h> // Structs needed for internet and domain address.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// Server Specific Libraries
#include "server.h"

int sock_server(unsigned int port)
{
    // Vars needed for socket
    int sockfd, newsockfd, portno, n;
    socklen_t clilen;
    char buffer[256];
    // Socket Connection info into buffer
    struct sockaddr_in serv_addr, cli_addr;
    // Open a socket file descriptor
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        error("ERROR opening socket");
    // Zero out the struct
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // Assign portno the value of the port that passed as an arguement
    portno = port;
    // Setup the binding varibles
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // Bind port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) , 0)
        error("ERROR on binding");
    // Listen on socket for connections (5 maximum)
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    // Write the packet to the socket.
    n = write(newsockfd,"I got your message", 18);
    if (n < 0)
        error("Error writing to socket");

    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
