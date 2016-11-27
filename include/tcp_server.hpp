#ifndef TCP_SERVER
#define TCP_SERVER

//Boost includes needed for classs types
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
//Custom TCP connection handler
#include <tcp_connection.hpp>

//Option to manualy define port
#define _PORT 9777

using boost::asio::ip::tcp;

// Make sure there is a port accositated with the server
extern unsigned int get_port_input(void);

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service);
private:
    void start_accept(void);
    void handle_accept(tcp_connection::pointer new_connection,
        const boost::system::error_code& error);
    tcp::acceptor acceptor_;
};
#endif
