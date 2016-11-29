/**
 * @auth: Reese Russell
 * @desc: A TCP session for the LTC2983
 **/
#include <ltc2983_session.hpp>
/**
 * @desc: LTC2983 TCP session constructor
 * @param: [ltc2983*] pointer a a ltc2983 device class object
 **/
ltc2983_session::ltc2983_session(ltc2983& dev) :
    dev_(dev), acceptor_(io_service
{
}

/**
 * @desc: Class Destructor
 **/
ltc2983_session::~ltc2983_session(void)
{
}
    
