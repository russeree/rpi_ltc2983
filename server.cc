// C++ LIBS
#include <iostream>
#include <bitset>
#include <cmath>
#include <fstream>
#include <thread>
#include <queue>

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

/**
 * @Desc: Sockets Mux is a transactional layer that is design to contol transaction requests by type
 * @Return: Returns the next state for the server to be in; 
 * @Param: [input_buffer] Pointer to the input buffer from the endpoint tcp socket
 * @Param: [output_buffer] Pointer to the output buffer going to the endpoint tcp socket 
 * @Param: [ib_size] Size of the input buffer
 * @Param: [ob_size] size of the output buffer
 * @Param: [state] current state of the server
 * @Note: Idealy this mux is meant to run as it's own thead controling the flow of data to and from the socket layer
 **/
int sokets_mux(char *input_buffer, char *output_buffer)
{
    // Convert the incoming message into a string
    std::string rx_parse(input_buffer);
    std::string state_string;
    // Now Case the string into a function call for the LTC2983
    if (rx_parse == "POLLING MODE")
    {
        state_string = "POLLING";
        strcpy(output_buffer,state_string.c_str());
        #ifdef SERV_DEBUG_1
        std::cout << "Next TCP response = ";
        for(int i = 0; i < 7; i++)
            std::cout << output_buffer[i];
        std::cout << "\n"; 
        #endif
        return 1;
    }
    if(rx_parse == "SPI_CHANNEL")
    {
        state_string = "SPI_CHANNEL";
        strcpy(output_buffer, state_string.c_str());
        return 2; 
    }
    // A Return Value of -1 Mean the mux did not perform any new operation. 
    return -1; 
}

/**
 * @desc: Communicates the channel configuration to Node Server.
 * @param: output_buffer.  Tx output buffer
 **/
int serv_get_channel_config(void) 
{
    return 0;
}

/**
 * @desc: Takes in a mesage and outputs a perror
 **/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
