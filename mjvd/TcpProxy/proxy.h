#ifndef INCLUDED_PROXY_HEADER
#define INCLUDED_PROXY_HEADER


#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>


///////////////////////////////////////////////////////////////////////////////////////////////////
struct ProxyParameters
{
    std::string listen_addr;
    std::string listen_port;
    std::string dest_addr;
    std::string dest_port;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class Proxy : private boost::noncopyable
{
public:
    Proxy(boost::asio::io_service& io_service, ProxyParameters const& params);
    ~Proxy();

    auto Start() -> void;
    auto Stop()  -> void;

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
};


#endif  //INCLUDED_PROXY_HEADER

