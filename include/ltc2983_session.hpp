#include <iostream>
#include <memory> 
#include <ltc2983.hpp>

class ltc2983_session :
    public std::enable_shared_from_this<ltc2983_session>
{
public:
    ltc2983_session();
    virtual ~ltc2983_session();
private:
    // Pointer to the ltc2983 to be used during this session
    ltc2983* dev; 
}
