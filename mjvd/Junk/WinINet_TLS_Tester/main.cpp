#include <iostream>
#include <string>
#include <cassert>
#include <sstream>
#include <vector>

#pragma comment(lib, "wininet.lib")
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinInet.h>
#include <conio.h>


namespace
{
    //=============================================================================================
    class InternetHandle
    {
    public:
        //canonical methods
        explicit InternetHandle(HINTERNET h = 0) : h_(h) {}
        InternetHandle(InternetHandle& rhs) : h_(rhs.h_) { rhs.h_ = 0; }
        ~InternetHandle() { reset(); }
        InternetHandle& operator= (InternetHandle& rhs) { reset(rhs.h_); rhs.h_ = 0; return *this; }

        void reset(HINTERNET h = 0) { if (h_) { if (!InternetCloseHandle(h_)) { assert(false); } } h_ = h; }
        operator HINTERNET() const { return h_; }

    private:
        HINTERNET h_;
    };

    //=============================================================================================
    std::string DoTransaction(InternetHandle const& hConnect, std::string const& protocol, std::string const& server, std::string const& service, std::string const& request)
    {
        // create an HTTP request handle
        static PCSTR acceptTypes[] = { "text/xml", 0 };

        // basic flags
        DWORD flags =
            INTERNET_FLAG_NO_CACHE_WRITE            |
            INTERNET_FLAG_RELOAD;

        // SSL?
        if (protocol == "https")
        {
            flags |= INTERNET_FLAG_SECURE                    | 
                     INTERNET_FLAG_IGNORE_CERT_CN_INVALID    | 
                     INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
        }

        // despite what the MSDN documentation says, we DO need to create a new hRequest handle for every send request.  If you don't, then 
        // things will almost work correctly, except that the 'Content-Length' header is not updated for subsequent requests...
        InternetHandle hRequest(
            HttpOpenRequest(
            hConnect,                               // InternetConnect handle
            "POST",                                 // method
            service.c_str(),                        // service object
            0,                                      // version
            0,                                      // referrer
            acceptTypes,                            // accept types
            flags,                                  // flags
            0));                                    // context
        if (!hRequest)
        {
            std::ostringstream oss;
            oss << "ERROR: unable to create an HTTPS request handle.  HttpOpenRequest() returned error [" << GetLastError() << "]";
            throw std::runtime_error(oss.str());
        }

        // add some extra headers - note that 'Content-Length' is added automatically
        std::ostringstream extraHeaders;
        extraHeaders
            << "Host: " << server                                   << "\r\n"
            << "Connection: Keep-Alive"                             << "\r\n"
            << "Cache-Control:  no-cache"                           << "\r\n"
            //<< "Content-Type: multipart/form-data"                  << "\r\n"
            //<< "Content-Type: application/x-www-form-urlencoded"    << "\r\n"
            << "Content-Type: application/xml"                      << "\r\n"
            ;
        if (!HttpAddRequestHeaders(hRequest, extraHeaders.str().c_str(), static_cast<DWORD>(-1), 0))
        {
            std::ostringstream oss;
            oss << "ERROR: unable to specify the extra headers.  HttpAddRequstHeaders() returned error [" << GetLastError() << "]";
            throw std::runtime_error(oss.str());
        }

        // send the request
        Again:
        BOOL result = 
            HttpSendRequest(
            hRequest,                               //HttpOpenRequest handle
            0,                                      //additional headers
            0,                                      //additional header length
            (LPVOID) request.c_str(),               //optional data (for POST and PUT requests)
            static_cast<DWORD>(request.size()));    //optional data length
        if (!result)
        {
            DWORD error = GetLastError();
            if (error == ERROR_INTERNET_INVALID_CA)
            {
                // the server's SSL certificate has been issued by an unknown or invalid certificate authority.  the correct thing 
                // to do here is to install the certificate authority's root certificate.
                //
                // As this is just an internal test app, i'm going to bypass the error instead.

                // read in our current security options flags
                DWORD flags;
                DWORD flagsSize = sizeof(flags);
                InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, &flagsSize);

                // modify the flags and set it
                flags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
                InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, sizeof(flags));
                goto Again;
            }
            else if (error == ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED)
            {
                // i think the server has requested - but does not require - a client certificate from us.  If we simply 
                // try the request again, it will probably work...
                goto Again;
            }
            else
            {
                std::ostringstream oss;
                oss << "ERROR: failed to send the request.  HttpSendRequest() returned error [" << error << "]";
                throw std::runtime_error(oss.str());
            }
        }

        // get the response
        char buffer[4096];
        std::ostringstream response;
        for (;;)
        {
            DWORD number_of_bytes_read;
            BOOL result = 
                InternetReadFile(
                    hRequest,
                    buffer,
                    sizeof (buffer) - 1,
                    &number_of_bytes_read);
            if (!result)
            {
                std::stringstream oss;
                oss << "ERROR: failed to get the response.  InternetReadFile() returned error [" << GetLastError() << "]";
                throw std::runtime_error(oss.str());
            }
            else if (number_of_bytes_read == 0)
            {
                break;
            }
            buffer[number_of_bytes_read] = '\0';
            response << buffer;
        }

        // for debugging purposes...
        /*
        std::cout 
            << "DoTransaction:"
            << "\n"
            << "\n[" << request  << "]"
            << "\n"
            << "\n[" << response.str() << "]"
            << "\n\n"
            << std::endl;
        */

        return response.str();
    }

    //=============================================================================================
    void InitialiseWinINet(std::string const& server, std::string const& port, InternetHandle& hInternet, InternetHandle& hConnect)
    {
        // Initialise WinINet
        hInternet.reset(InternetOpen("FirstDataTestScripts", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0));
        if (!hInternet)
        {
            std::ostringstream oss;
            oss << "ERROR: unable to start WinINET. InternetOpen() returned error [" << GetLastError() << "]";
            throw std::runtime_error(oss.str());
        }

        // open an HTTP session with the remote server
        INTERNET_PORT const port2 = static_cast<INTERNET_PORT>(std::stoul(port));

        hConnect.reset(
            InternetConnect(
            hInternet,                      // InternetOpen handle
            server.c_str(),                 // remote url
            port2,                          // remote port
            0,                              // username
            0,                              // password
            INTERNET_SERVICE_HTTP,          // type of service
            0,                              // flags
            0));                            // context
        if (!hConnect)
        {
            std::ostringstream oss;
            oss << "ERROR: unable to connect to remote server [" << server << "].  InternetConnect() returned error [" << GetLastError() << "]";
            throw std::runtime_error(oss.str());
        }
    }
}

typedef int blah[8];

auto foo(blah *x) -> void
{
}

//=================================================================================================
auto main() -> int
{
    std::vector<int> input = { 1, 2, 3, 4, 5, 6, 7, 8 };
    int* p = &input[0];

    foo(reinterpret_cast<blah*>(p));
    return 0;










    try
    {
        //
        //  https://migs-mtf.mastercard.com.au/vpcdps
        //
        std::string const protocol  = "https";
        std::string const server    = "migs-mtf.mastercard.com.au";
        std::string const port      = "443";
        std::string const service   = "vpcdps";

        // initialise WinINet
        InternetHandle  hInternet;
        InternetHandle  hConnect;
        InitialiseWinINet(server, port, hInternet, hConnect);

        // create our request
        auto const request = std::string{ "blah" };
        std::cout << request << std::endl;

        // do the transaction
        auto const response = DoTransaction(hConnect, protocol, server, service, request);
        std::cout << response << std::endl;

        // all done
        std::cout << "\n\n\nPress any key to exit..." << std::endl;
        _getch();
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        std::cout << "main():  std::exception caught:  " << e.what() << std::endl;
        std::cout << "\n\n\nPress any key to exit..." << std::endl;
        _getch();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "main():  unknown exception caught" << std::endl;
        std::cout << "\n\n\nPress any key to exit..." << std::endl;
        _getch();
        return EXIT_FAILURE;
    }
}