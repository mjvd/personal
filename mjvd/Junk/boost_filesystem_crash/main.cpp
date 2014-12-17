#include <boost/filesystem.hpp>
#include <future>

namespace
{
    // workaround for boost::filesystem bug ticket #6320.  This dummy path initialises 
    // a function local static in a thread-safe manner.
    boost::filesystem::path dummy("");
}

int main(int argc, char** argv)
{
    auto f = []() { boost::filesystem::path(""); };
    std::async(f);
    std::async(f);
}

