/**
 * @auth: Reese Russell
 * @desc: A TCP session for the LTC2983
 **/
#include <ltc2983_session.hpp>
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
 * @desc: LTC2983 TCP session constructor
 * @param: [ltc2983*] pointer a a ltc2983 device class object
 **/
ltc2983_session::ltc2983_session(ltc2983& dev, boost::asio::io_service& io_service) :
    dev_(dev), acceptor_(io_service, tcp::endpoint(tcp::v4(), get_port_input()))
{
    start_accept(); //Start accepting connections on the current endpoint;
}

/**
 * @desc: Class Destructor
 **/
ltc2983_session::~ltc2983_session(void)
{
#ifdef _VERBOSE
    std::cout << "Destroyed a LTC2983 session";
#endif
}

/**
 * @desc: Starts an individual TCP session 
 **/ 
void ltc2983_session::start_accept()
{
}
