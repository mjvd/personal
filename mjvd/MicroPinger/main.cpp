#include "stdafx.h"
#include <iostream>
#include <cstddef>
#include <cassert>
#include <exception>
#include <boost/asio.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "Pinger.h"


//=================================================================================================
int main(int argc, char* argv[])
{
    try
    {
        int  maxHistogramValue;
        int  idealCutoff;
        int  pingPeriod;
        int  statsPeriod;
        std::string dest;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help",                                                                            "produce help message")
            ("verbose",                                                                         "display the RTT of each packet")
            ("precise",                                                                         "display the RTT of each packet down to the micro-second")
            ("dest",                po::value<std::string>(&dest),                              "hostname or IPv4 address of destination")
            ("max-histogram-value", po::value<int> (&maxHistogramValue) ->default_value(400),   "must be one of {200,400,600,800,1000}")
            ("ideal-cutoff",        po::value<int> (&idealCutoff)       ->default_value(80),    "displays percentage of all packets with RTT lower than this cutoff")
            ("ping-period",         po::value<int> (&pingPeriod)        ->default_value(200),   "how often to send each ping packet (in milli-seconds)")
            ("stats-period",        po::value<int> (&statsPeriod)       ->default_value(10),    "how often to produce the statistics (in minutes)")
            ;

        // we will support one and only 1 positional option - which is the destination
        po::positional_options_description pd;
        pd.add("dest", 1);

        // process the command line
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
        po::notify(vm);

        if (argc <= 1 || vm.count("help"))
        {
            std::cout
                << "\n                                                                           "
                << "\nMicroPinger                                                                "
                << "\n                                                                           "
                << "\n" << desc
                << "\n                                                                           "
                << "\n                                                                           "
                << "\n  TIMING PRECISION:                                                        " 
                << "\n  This application was specifically designed to calculate the RTT as       "
                << "\n  accurately as possible.  This app uses the QueryPerformanceCounter API,  "
                << "\n  and as such, the timestamps that are generated when we send and receive  "
                << "\n  each packet should be accurate to within a few of micro-seconds.  The    "
                << "\n  app is based on the Boost.Asio network library, and packets are handled  " 
                << "\n  asynchronously.  There is no polling here to introduce any delays.       "
                << "\n                                                                           "
                << "\n  Note that these 'micro-second accurate' RTT for the ping packets will    "
                << "\n  of course include delays due to the packet traversing this app, the local"
                << "\n  network stack, and other delays due to the operating system itself.      "
                << "\n                                                                           "
                << "\n                                                                           "
                << "\n  NOTES:                                                                   "
                << "\n    * If any packet takes more than 1 second to come back, it will be      "
                << "\n      considered to have timed-out.                                        "
                << "\n    * This app sends only a single packet at any time, then waits for up   "
                << "\n      to 1 second for the response.                                        "
                << "\n    * Packets are throttled to a maximum rate as specified on the command  "
                << "\n      line in the <pingPeriod> parameter.                                  "
                << "\n    * For best viewing, maximize your console window.                      "
                << "\n    * This application uses raw sockets, therefore it must be run with     "
                << "\n      administrator privileges.                                            "
                << "\n                                                                           "
                << "\n                                                                           "
                << "\n  PROTOCOL SUPPORT:                                                        "
                << "\n  This application supports ICMP over IPv4.  It does not support IPv6.     "
                << "\n                                                                           "
                << "\n                                                                           "
                << std::endl;
            return EXIT_FAILURE;
        }

        if (dest.empty())                           { std::cout << "ERROR:  you must specify a destination" << std::endl; return EXIT_FAILURE; }
        if (idealCutoff < 1 || idealCutoff > 1000)  { std::cout << "ERROR:  ideal-cutoff is invalid"        << std::endl; return EXIT_FAILURE; }
        if (pingPeriod < 1 || pingPeriod > 60000)   { std::cout << "ERROR:  ping-period is invalid"         << std::endl; return EXIT_FAILURE; }
        if (maxHistogramValue < 200 || maxHistogramValue > 1000 || (maxHistogramValue % 200 != 0)) { std::cout << "ERROR:  max-histogram-value is invalid" << std::endl;  return EXIT_FAILURE; }

        auto verbose = vm.count("verbose") == 1;
        auto precise = vm.count("precise") == 1;

        boost::asio::io_service io_service;
        mvd::Pinger pinger(io_service, dest, pingPeriod, statsPeriod, verbose, precise, maxHistogramValue, idealCutoff);
        io_service.run();
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        std::cout << "ERROR:  std::exception caught in main():  " << e.what() << std::endl;
        assert(false);
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "ERROR:  unknown exception caught in main()" << std::endl;
        assert(false);
        return EXIT_FAILURE;
    }
}

