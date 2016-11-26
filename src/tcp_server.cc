/**
 * @auth: Reese Russell
 * @date: 11/26/2016
 * @desc: A tcp server implementation
 **/

// C++ LIBS
#include <iostream>
#include <cstdlib>
// Server Specific Libraries
#include <tcp_server.hpp>

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
}

tcp_server::tcp_server(boost::asio::io_service& io_service) : acceptor_(io_service, tcp::endpoint(tcp::v4(), get_port_input()))
{
    start_accept();
}

void tcp_server::start_accept()
{
// Create a pointer to the new connection
    tcp_connection::pointer new_connection = tcp_connection::create(acceptor_.io_service());
// Bind a handler to the connection
}
