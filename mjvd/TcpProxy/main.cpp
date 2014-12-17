#include "stdafx.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <memory>

#include <boost/asio/io_service.hpp>
namespace ba = boost::asio;

#include "proxy.h"
#include "http_client.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayUsage()
{
    std::cout
        << "\nUsage:  TcpProxy <listen_addr> <listen_port> <dest_addr> <dest_port>"
        << "\n"
        << "\n    This application listens on the specified TCP port.  When a connection is"
        << "\n    established on this port, it will connect to the specified destination."
        << "\n    All data is proxied in both directions.  When either side closes the "
        << "\n    connection, then TcpProxy will forcibly close the remaining connection."
        << "\n"
        << "\n    TcpProxy will only accept a single incomming connection at any one time - "
        << "\n    i.e. it does not multiplex connections."
        << "\n"
        << "\n    This app fully supports IPv6."
        << "\n"
        << "\n        TcpProxy ::0 81 ::1 80"
        << "\n"
        << "\n"
        << std::endl;

    return EXIT_FAILURE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    try
    {
        // ensure we are running in x64 mode
        assert(sizeof(void*) == 8);

        // our one and only io_service
        ba::io_service io_service;

        // get the command-line params
        if (argc == 3)
        {
            HttpClientParameters params;
            params.listen_addr = argv[1];
            params.listen_port = argv[2];

            auto httpClient = std::make_shared<HttpClient>(io_service, params);
            httpClient->Start();
        }
        else if (argc == 5)
        {
            ProxyParameters params;
            params.listen_addr = argv[1];
            params.listen_port = argv[2];
            params.dest_addr   = argv[3];
            params.dest_port   = argv[4];

            // create our proxy object
            auto proxy = std::make_shared<Proxy>(io_service, params);
            proxy->Start();
        }
        else
        {
            return DisplayUsage();
        }

        // run to completion
        io_service.run();
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        assert(false);
        std::cout << "main() - ERROR: caught a std::exception:  " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        assert(false);
        std::cout << "main() - ERROR: caught an unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
}

