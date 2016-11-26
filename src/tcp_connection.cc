/**
 * @auth: Reese Russell
 * @date: 11/26/2016
 * @desc: TCP connection handler functions
 **/

// This will include all the boost libs needed for this class
#include <tcp_connection.hpp>

tcp_connection::pointer tcp_connection::create(boost::asio::io_service& io_service)
{
    return pointer(new tcp_connection(io_service));
}
