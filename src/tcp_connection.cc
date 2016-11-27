/**
 * @auth: Reese Russell
 * @date: 11/26/2016
 * @desc: TCP connection handler functions
 **/

// This will include all the boost libs needed for this class
#include <tcp_connection.hpp>

// Create a pointer to a new TCP connection
tcp_connection::pointer tcp_connection::create(boost::asio::io_service& io_service)
{
    return pointer(new tcp_connection(io_service));
}

// Return the private socket varible
tcp::socket& tcp_connection::socket(void)
{
    return socket_;
}

// The task to be completed upon a connection
void tcp_connection::start()
{
    message_ = "TEST";
    boost::asio::async_write(socket_, boost::asio::buffer(message_),
        boost::bind(&tcp_connection::handle_write, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

// Class Constructor
tcp_connection::tcp_connection(boost::asio::io_service& io_service) : socket_(io_service)
{
}

// Connection handler
void tcp_connection::handle_write(const boost::system::error_code&, size_t)
{
}
