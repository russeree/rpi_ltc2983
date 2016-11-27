#ifndef TCP_CONNECTION
#define TCP_CONNECTION

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;
    static pointer create(boost::asio::io_service& io_service);
    tcp::socket& socket(void);
    void start();
private:
    tcp_connection(boost::asio::io_service& io_service);
    void handle_write(const boost::system::error_code& ,size_t);
    tcp::socket socket_;
    std::string message_;
};
#endif
