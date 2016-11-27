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

/**
 * @desc: is there is no port defined for the TCP server, prompt the user to enter one.
 * @param: No parameters
 **/
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

/**
 * @desc: TCP Server constructor
 **/
tcp_server::tcp_server(boost::asio::io_service& io_service) : acceptor_(io_service, tcp::endpoint(tcp::v4(), get_port_input()))
{
    start_accept();
}

/**
 * @desc: Accepts a connection
 **/
void tcp_server::start_accept()
{
    // Create a pointer to the new connection
    tcp_connection::pointer new_connection = 
        tcp_connection::create(acceptor_.get_io_service());
    // Bind a handler to the connection
    acceptor_.async_accept(new_connection -> socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
}

/**
 * @desc: Connection acceptance handler
 **/
void tcp_server::handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
{
    if(!error)
        new_connection -> start();
    start_accept();
}
