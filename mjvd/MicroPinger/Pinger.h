#pragma once

#include <string>
#include <memory>

namespace boost { namespace asio { class io_service; } }


namespace mvd
{
    class Pinger
    {
    public:
        Pinger(boost::asio::io_service& io_service, std::string const& destination, int pingPeriod, int statsPeriod, bool verbose, bool precise, int maxHistogramValue, int idealCutoff);
        ~Pinger();

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}

