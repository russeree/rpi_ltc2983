#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <ltc2983.hpp>

using boost::asio::ip::tcp;

class ltc2983_session :
    public std::enable_shared_from_this<ltc2983_session>
{
public:
    ltc2983_session(ltc2983& dev, boost::asio::io_service& io_service);
    virtual ~ltc2983_session();
private:
    // Pointer to the ltc2983 to be used during this session
    tcp::acceptor acceptor_;
    ltc2983* dev_; 
};
