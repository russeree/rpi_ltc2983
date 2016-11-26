#ifndef SERVER
#define SERVER

//Boost includes needed for classs types
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

//Option to manualy define port
#define _PORT 9777

using boost::asio::ip::tcp;

extern unsigned int get_port_input(void);

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service);
private:
    tcp::acceptor acceptor_;
};
#endif
