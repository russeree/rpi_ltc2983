// C++ LIBS
#include <iostream>
#include <cstdlib>

// Server Specific Libraries
#include "server.h"

extern unsigned int get_port_input(void)
{
    unsigned int port;
#ifdef _PORT
    port = _PORT;
#else
    std::cout << "Enter a port number for the TCP server ";
    std::cin  >> port;
    try
    {
        if(port > 65535)
            throw port;
    }
    catch(unsigned int port)
    {
        std::cout << "Port " << port << " is invalid. Port set to 0";
        port = 0;
    }
#endif 
    return port;
};


tcp_server::tcp_server(boost::asio::io_service& io_service) : acceptor_(io_service, tcp::endpoint(tcp::v4(), get_port_input()))
{
};

