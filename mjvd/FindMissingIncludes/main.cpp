#include <iostream>
#include <cstddef>
#include <cassert>
#include <regex>
#include <boost/algorithm/string/predicate.hpp>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
namespace prop = boost::property_tree;


//=================================================================================================
void DisplayUsage()
{
    std::cout
        << "\n"
        << "\n"
        << "\nUsage:  FindDanglingIncludes <dir|sln|vcxproj>"
        << "\n"
        << "\n    Author:  Mark van Dijk"
        << "\n"
        << "\n    This app requires a single command-line argument:"
        << "\n        <vcxproj> : the specified project file will be processed"
        << "\n        <sln>     : the specified solution file will be processed"
        << "\n        <dir>     : all solution and project files in the current directory (and"
        << "\n                    all sub-directories) will be processed"
        << "\n"
        << "\n    Each project file is scanned, the INCLUDE files identified, and the app then"
        << "\n    verifies the existence of those files.  Any dangling references are flagged,"
        << "\n    and the appropriate %ERRORLEVEL% is returned. "
        << "\n"
        << "\n"
        << "\n";
}

//=================================================================================================
bool ProcessProjectFile(fs::path const& path)
{
    bool foundError = false;
    try
    {
        fs::path const currentDir = path.branch_path();
        std::cout 
            << "\nprocessing [" << path.string() << "]"
            << std::flush;

        // load in the vcxproj
        std::ifstream fileStream(path.string());
        prop::ptree pt;
        read_xml(fileStream, pt);
        //std::cout << "\n    successfully loaded vcxproj" << std::flush;

        /*
        auto const& projectNode = pt.get_child_optional("Project");
        if (projectNode)
        {
            for (auto const& subtree : projectNode.get())
            {
                if (subtree.first == "ItemGroup")
                {
                    for (auto const& includeAttribute : subtree.second.get_child("ClInclude.<xmlattr>.Include"))
                    {
                        auto const& name = includeAttribute.first;
                        int i=0;
                    }
                }
            }
        }
        */

        // iterate over all the tags we are after
        //
        // TODO:  I'm sure there is a MUCH easier way for me to extract the info, however the code below works for me, and i just don't have
        //        time at the moment to do something more elegant...
        //
        for (auto const& node1 : pt)
        {
            auto const& name    = node1.first;
            auto const& value   = node1.second.data();
            auto const& subtree = node1.second;
        
            if (name == "Project")
            {
                //std::cout << "\n        <Project> node" << std::flush;
                for (auto const& node2 : subtree)
                {
                    auto const& name    = node2.first;
                    auto const& value   = node2.second.data();
                    auto const& subtree = node2.second;

                    if (name == "ItemGroup")
                    {
                        //std::cout << "\n            <ItemGroup>" << std::flush;
                        for (auto const& node3 : subtree)
                        {
                            auto const& name    = node3.first;
                            auto const& value   = node3.second.data();
                            auto const& subtree = node3.second;

                            if (name == "ClInclude")
                            {
                                //std::cout << "\n                <ClInclude>" << std::flush;
                                for (auto const& node4 : subtree)
                                {
                                    auto const& name    = node4.first;
                                    auto const& value   = node4.second.data();
                                    auto const& subtree = node4.second;

                                    if (name == "<xmlattr>")
                                    {
                                        for (auto const& node5 : subtree)
                                        {
                                            auto const& name    = node5.first;
                                            auto const& value   = node5.second.data();
                                            auto const& subtree = node5.second;

                                            if (name == "Include")
                                            {
                                                auto const exists = fs::exists(currentDir / value) && fs::is_regular_file(currentDir / value);
                                                if (!exists)
                                                {
                                                    std::cout << "\n   ****  dangling include:    [" << value << "]";
                                                    foundError = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (foundError)
        {
            std::cout << "\n";
        }
        else
        {
            //std::cout << "\n    successfully found all includes" << std::flush;
        }

    }
    catch (std::exception& e)
    {
        std::cout 
            << "\nERROR:  while processing project file:"
            << "\n    file:  [" << path.string() << "]"
            << "\n    what:  " << e.what()
            << std::endl;

        foundError = true;
    }

    return foundError;
}

//=================================================================================================
bool ProcessSolutionFile(fs::path const& path)
{
    std::cout << "\nprocessing solution file:  [" << path.string() << "]";
    std::ifstream f(path.string());

    bool foundError = false;
    std::string line;
    while (std::getline(f, line))
    {
        static std::regex const regex("^Project\\(\"\\{[\\-[:xdigit:]]{36}\\}\"\\) = \"(.*)\", \"(.*)\", \"\\{[\\-[:xdigit:]]{36}\\}\"$");
        std::smatch m;
        if (std::regex_match(line, m, regex))
        {
            auto const projectName = m[1];
            auto const projectFile = m[2];
            auto const projectFilePath = path.branch_path() / projectFile.str();

            if (fs::exists(projectFilePath) && fs::is_regular_file(projectFilePath) && fs::extension(projectFilePath) == ".vcxproj")
            {
                foundError |= ProcessProjectFile(projectFilePath);
            }
            else
            {
                if (projectName != "Solution Items")
                {
                    std::cout << "\n    skipping project found in solution file:"
                        << "\n        project name:  [" << projectName << "]"
                        << "\n        project file:  [" << projectFilePath.string() << "]"
                        << "\n";
                }
            }
        }
    }

    return foundError;
}

//=================================================================================================
bool FindAllSolutionAndProjectFiles(fs::path const& root)
{
    bool foundErrors = false;
    for (fs::recursive_directory_iterator itr(root); itr != fs::recursive_directory_iterator(); ++itr)
    {
        if (fs::is_regular_file(*itr))
        {
            if (boost::iequals(fs::extension(*itr), ".vcxproj"))
            {
                foundErrors |= ProcessProjectFile(*itr);
            }
            else if (boost::iequals(fs::extension(*itr), ".sln"))
            {
                foundErrors |= ProcessSolutionFile(*itr);
            }
        }
    }
    return foundErrors;
}

//=================================================================================================
int main(int argc, char** argv)
{
    try
    {
        fs::path root;
        bool foundErrors = false;

        DisplayUsage();
        if (argc == 2)
        {
            fs::path const root(argv[1]);
            if (fs::is_directory(root))
            {
                // search (recursively) for all project files in the specified directory
                foundErrors = FindAllSolutionAndProjectFiles(root);            
            }
            else if (fs::is_regular_file(root))
            {
                if (boost::iequals(fs::extension(root), ".vcxproj"))
                {
                    // just process the specified project file
                    foundErrors = ProcessProjectFile(root);
                }
                else if (boost::iequals(fs::extension(root), ".sln"))
                {
                    // process the specified solution file
                    foundErrors = ProcessSolutionFile(root);
                }
                else
                {
                    std::cout << "ERROR:  invalid argument:  [" << root << "]" << std::endl;
                }
            }
            else
            {
                assert(false);
            }
        }

        std::cout << "\n\n";

        if (foundErrors)
        {
            std::cout 
                << "\n****************************************************************************"
                << "\n**  Dangling INCLUDE references were found in one (or more) projects.     **"
                << "\n**                                                                        **"
                << "\n**  The logging above will indicate the problem projects and specific     **"
                << "\n**  include files.  Delete the reference to these include files from the  **"
                << "\n**  project(s).                                                           **"
                << "\n****************************************************************************"
                << "\n\n";
        }

        return foundErrors ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    catch (fs::filesystem_error& e)
    {
        std::cout 
            << "\n\nmain():  boost::filesystem_error caught:"
            << "\n    what:   [" << e.what() << "]"
            << "\n    code:   [" << e.code() << "]"
            << "\n    path1:  [" << e.path1() << "]"
            << "\n    path2:  [" << e.path2() << "]"
            << "\n"
            << std::endl;
        assert(false);
        return EXIT_FAILURE;
    }
    catch (std::exception& e)
    {
        std::cout << "\n\nmain():  std::exception caught:  " << e.what() << std::endl;
        assert(false);
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "\n\nmain():  unknown exception caught" << std::endl;
        assert(false);
        return EXIT_FAILURE;
    }
}
