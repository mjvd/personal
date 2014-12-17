#include "stdafx.h"
#include "proxy.h"
#include <memory>

#include <boost/asio.hpp>
namespace ba = boost::asio;

#include <boost/date_time/posix_time/posix_time.hpp>
namespace pt = boost::posix_time;

#include <boost/algorithm/string/replace.hpp>


namespace
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto TimeStamp() -> std::string
    {
        std::string t = pt::to_simple_string(pt::microsec_clock::local_time().time_of_day());
        t.resize(12, '0');
        return t.append(1, ' ');
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
class Proxy::Impl : public std::enable_shared_from_this<Impl>
{
public:
    Impl(Proxy* self, ba::io_service& io_service, ProxyParameters const& params);
    auto Start() -> void;
    auto Stop()  -> void;

private:
    auto HandleStop() -> void;
    auto EstablishNewConnections() -> void;

    auto HandleDestRead     (boost::system::error_code const& error, std::size_t bytes_transferred) -> void;
    auto HandleListenRead   (boost::system::error_code const& error, std::size_t bytes_transferred) -> void;
    auto HandleDestWrite    (boost::system::error_code const& error)                                -> void;
    auto HandleListenWrite  (boost::system::error_code const& error)                                -> void;

    //auto StartAccept() -> void;
    //auto HandleAccept(boost::system::error_code const& error) -> void;

private:
    Proxy*          const   mSelf;
    ProxyParameters const   mParams;

    ba::io_service&         mIoService;
    ba::signal_set          mSignals;
    ba::ip::tcp::acceptor   mAcceptor;
    ba::ip::tcp::socket     mListenSocket;
    ba::ip::tcp::socket     mDestSocket;

    bool                    mClosingListenSocket;
    bool                    mClosingDestSocket;
    bool                    mShuttingDown;

    static int const        LENGTH_ = 16*1024;
    char                    mListenData[LENGTH_];
    char                    mDestData[LENGTH_];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
Proxy::Impl::Impl(Proxy* self,  ba::io_service& io_service, ProxyParameters const& params)
    : mSelf         {self}
    , mIoService    {io_service}
    , mParams       {params}
    , mSignals      {io_service}
    , mAcceptor     {io_service}
    , mListenSocket {io_service}
    , mDestSocket   {io_service}
    , mShuttingDown {false}
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto Proxy::Impl::Start() -> void
{
    // register to handle the stop/terminate/quit signals
    mSignals.add(SIGINT);
    mSignals.add(SIGTERM);
#if defined (SIGQUIT)
    mSignals.add(SIGQUIT);
#endif
    mSignals.async_wait(std::bind(&Impl::HandleStop, shared_from_this()));

    EstablishNewConnections();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto Proxy::Impl::Stop() -> void
{
    mIoService.post(std::bind(&Impl::HandleStop, shared_from_this()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto Proxy::Impl::HandleStop() -> void
{
    std::cout << TimeStamp() << "shutting down TcpProxy" << std::endl;
    mShuttingDown = true;

    std::cout << TimeStamp() << "closing acceptor" << std::endl;
    mAcceptor.close();

    std::cout << TimeStamp() << "closing listening connection (if any)" << std::endl;
    mListenSocket.cancel();

    std::cout << TimeStamp() << "closing destination connection (if any)" << std::endl;
    mDestSocket.cancel();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// This method synchronously waits for a new incomming connection, then resolves and connects to the 
// destination connection.
//
auto Proxy::Impl::EstablishNewConnections() -> void
{
    try
    {
        mShuttingDown        = false;
        mClosingDestSocket   = false;
        mClosingListenSocket = false;


        {
            // resolve our listening address/port
            std::cout << TimeStamp() << "resolving listening address:  [" << mParams.listen_addr << "]:" << mParams.listen_port << std::endl;
            ba::ip::tcp::resolver           resolver(mIoService);
            ba::ip::tcp::resolver::query    query   (mParams.listen_addr, mParams.listen_port);
            ba::ip::tcp::endpoint           endpoint = *resolver.resolve(query);

            // open the acceptor with SO_REUSEADDR
            std::cout << TimeStamp() << "waiting for new incomming connection" << std::endl;
            ba::ip::tcp::acceptor acceptor(mIoService);
            acceptor.open(endpoint.protocol());
            acceptor.set_option(ba::ip::tcp::acceptor::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.listen();
            acceptor.accept(mListenSocket);

            // we have a new connection
            ba::ip::tcp::endpoint const& ep_local  = mListenSocket.local_endpoint();
            ba::ip::tcp::endpoint const& ep_remote = mListenSocket.remote_endpoint();
            std::cout << TimeStamp() << "new listening connection:    [" << ep_remote.address().to_string() << "]:" << ep_remote.port() << "  --->  [" << ep_local.address().to_string() << "]:" << ep_local.port() << std::endl;
        }



        {
            // resolve destination address/port
            std::cout << TimeStamp() << "resolving destination address:  [" << mParams.dest_addr << "]:" << mParams.dest_port << std::endl;
            ba::ip::tcp::resolver           resolver(mIoService);
            ba::ip::tcp::resolver::query    query   (mParams.dest_addr, mParams.dest_port);
            ba::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

            std::ostringstream oss;
            oss << std::hex << 1234;
            // establish connection
            ba::connect(mDestSocket, iterator);

            // we have a new connection
            ba::ip::tcp::endpoint const& ep_local  = mDestSocket.local_endpoint();
            ba::ip::tcp::endpoint const& ep_remote = mDestSocket.remote_endpoint();
            std::cout << TimeStamp() << "new destination connection:    [" << ep_local.address().to_string() << "]:" << ep_local.port() << "  --->  [" << ep_remote.address().to_string() << "]:" << ep_remote.port() << std::endl;
        }


        // finally kick off an async 'read' operation on both the listen and dest connections
        mListenSocket.async_read_some(
            ba::buffer(mListenData, LENGTH_),
            std::bind(&Impl::HandleListenRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

        mDestSocket.async_read_some(
            ba::buffer(mDestData, LENGTH_),
            std::bind(&Impl::HandleDestRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
    catch (std::exception& e)
    {
        std::cout << TimeStamp() << "ERROR:  EstablishNewConnection():  " << e.what() << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto Proxy::Impl::HandleListenRead(boost::system::error_code const& error, std::size_t bytes_transferred) -> void
{
    if (!error)
    {
        std::string request{mListenData, bytes_transferred};

        /*
        std::string const HOST_SRC = "Host: 192.168.100.142:12345\r\n";
        std::string const HOST_DST = "Host: 192.168.100.142:80\r\n";
        if (request.find(HOST_SRC) != std::string::npos)
        {
            boost::replace_first(request, HOST_SRC, HOST_DST);
            std::copy(begin(request), end(request), mListenData);
            bytes_transferred = request.size();
        }
        */

        std::cout << TimeStamp() << "client --> server:\n    |" << boost::replace_all_copy(request, "\n", "\n    |") << "\n" << std::endl;
        ba::async_write(
            mDestSocket, 
            ba::buffer(mListenData, bytes_transferred),
            std::bind(&Impl::HandleDestWrite, shared_from_this(), std::placeholders::_1));
    }
    else
    {
        if (mShuttingDown)
        {
            std::cout << TimeStamp() << "client socket successfully shut down:  " << error.message() << std::endl;
        }
        else if (mClosingListenSocket)
        {
            std::cout << TimeStamp() << "client socket successfully shut down:  " << error.message() << std::endl;
            EstablishNewConnections();
        }
        else
        {
            std::cout << TimeStamp() << "client has closed the connection:  " << error.message() << std::endl;
            mClosingDestSocket = true;
            mDestSocket.close();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto Proxy::Impl::HandleDestRead(boost::system::error_code const& error, std::size_t bytes_transferred) -> void
{
    if (!error)
    {
        std::string const request(mDestData, bytes_transferred);
        std::cout << TimeStamp() << "client <-- server:\n    |" << boost::replace_all_copy(request, "\n", "\n    |") << "\n" << std::endl;
        ba::async_write(
            mListenSocket, 
            ba::buffer(mDestData, bytes_transferred),
            std::bind(&Impl::HandleListenWrite, shared_from_this(), std::placeholders::_1));
    }
    else
    {
        if (mShuttingDown)
        {
            std::cout << TimeStamp() << "server socket successfully shut down:  " << error.message() << std::endl;
        }
        else if (mClosingDestSocket)
        {
            std::cout << TimeStamp() << "server socket successfully shut down:  " << error.message() << std::endl;
            EstablishNewConnections();
        }
        else
        {
            std::cout << TimeStamp() << "server has closed the connection:  " << error.message() << std::endl;
            mClosingListenSocket = true;
            mListenSocket.close();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto Proxy::Impl::HandleListenWrite(boost::system::error_code const& error) -> void
{
    if (!error)
    {
        // and kick off another read
        mDestSocket.async_read_some(
            ba::buffer(mDestData, LENGTH_), 
            std::bind(&Impl::HandleDestRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        if (!mShuttingDown)
        {
            std::cout << TimeStamp() << "WARNING:  HandleListenWrite():  failed:  " << error.message() << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto Proxy::Impl::HandleDestWrite(boost::system::error_code const& error) -> void
{
    if (!error)
    {
        // and kick off another read
        mListenSocket.async_read_some(
            ba::buffer(mListenData, LENGTH_), 
            std::bind(&Impl::HandleListenRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        if (!mShuttingDown)
        {
            std::cout << TimeStamp() << "ERROR:  HandleDestWrite():  failed:  " << error.message() << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Proxy::Proxy(ba::io_service& io_service, ProxyParameters const& params)
    : mImpl(std::make_shared<Impl>(this, io_service, params))
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Proxy::~Proxy()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto Proxy::Start() -> void
{
    mImpl->Start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto Proxy::Stop() -> void
{
    mImpl->Stop();
}


