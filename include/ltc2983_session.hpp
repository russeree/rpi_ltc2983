#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <ltc2983.hpp>

//New connection handler 
#include <tcp_connection.hpp>

#define _PORT 9777
#define _VERBOSE

using boost::asio::ip::tcp;
//Use in session constructor to obtain a port if one is not defined
extern unsigned int get_port_input(void);
/**
 * @desc: for now this represent a LTC2983 session, a session can have many clients but idealy only
 * one session per LTC2983 should exist. The session acts as a TCP server
 * @param: [ltc2983&] A reference to an ltc2983 class object that has been constructed.
 * @param: [io_service] A reference to an ASIO io service to communicate over. 
 **/
class ltc2983_session :
    public std::enable_shared_from_this<ltc2983_session>
{
public:
    ltc2983_session(ltc2983& dev, boost::asio::io_service& io_service);
    virtual ~ltc2983_session();
    void start_accept();
private:
    tcp::acceptor acceptor_;
    //Reference to the ltc2983 class, !!NO CPY/NEW!!
    ltc2983& dev_; 
};
