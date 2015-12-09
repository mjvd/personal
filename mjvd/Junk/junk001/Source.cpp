#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

int main()
{
    auto const path = fs::path{ "f:/PXLOG/PXUPLINKLIB_UNITTESTS" };
    auto const exists = fs::exists(path);
    return 0;
}