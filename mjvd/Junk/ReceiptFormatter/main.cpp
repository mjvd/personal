#include <iostream>
#include <fstream>
#include <string>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


//=================================================================================================
auto DisplayUsage() -> int
{
    std::cout 
        << "\nUsage:  ReceiptFormatter <input>"
        << "\n"
        << "\n    This utility application reads in a text file containing EFTPOS receipts"
        << "\n    on a single line - possibly with trailing SCR commands.  It will strip"
        << "\n    off the trailing rubbish, then nicely format each receipt into a new"
        << "\n    modified text file."
        << "\n"
        << "\n"
        << std::endl;

    return EXIT_FAILURE;
}

//=================================================================================================
int main(int argc, char** argv)
{
    try
    {
        // open the file
        if (argc != 2) { return DisplayUsage(); }
        auto const f = fs::path{ argv[1] };
        if (!fs::is_regular_file(f)) { return DisplayUsage(); }

        // slurp in the contents of the file
        std::cout << "reading file [" << f.string() << "]" << std::endl;
        auto lines = std::vector<std::string>{};
        std::ifstream inFile(f.string());
        std::string line;
        while (std::getline(inFile, line))
        {
            lines.push_back(line);
        }
        // std::copy(std::istream_iterator<std::string>(inFile), std::istream_iterator<std::string>(), std::back_inserter(lines));
     
        // write modified contents out to new file
        auto const fNew = f.branch_path() / (f.filename().string() + "_modified" + f.extension().string());
        std::cout << "writing new file [" << fNew << "]" << std::endl;
        std::ofstream outFile(fNew.string(), std::ios::trunc);
        for (auto const& line : lines)
        {
            // strip off everything from the first '~' character onwards, and verify that length is a mulitple of 30
            auto ll = line.substr(0, line.find('~'));
            if (ll.size() % 30 != 0)
            {
                assert(false);
                std::cout << "following line is not a multiple of 30:  [" << line << "]";
                throw std::runtime_error("line length error");
            }

            // our prefix
            outFile
                << "\n****************************************************************************"
                << "\n**"
                << "\n**"
                << "\n";

            // the line itself - broken up into chunks of 30
            while (!ll.empty())
            {
                outFile << "\n" << ll.substr(0, 30);
                ll = ll.substr(30);
            }

            // our suffix
            outFile 
                << "\n"
                << "\n"
                << "\n"
                << "\n"
                << "\n"
                << "\n";
        }

        std::cout << "finished successfully" << std::endl;
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        std::cout << "main():  std::exception caught:  " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "main():  unknown exception caught" << std::endl;
        return EXIT_FAILURE;
    }
}