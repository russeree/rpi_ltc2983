#ifndef SERVER
#define SERVER

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

extern boost::asio::io_service ioservice; 

class session
{
public:
};
#endif
