#pragma once

#include <string>
#include <memory>
#include <vector>
#include <boost/filesystem/path.hpp>

// forward declarations.  Note that VS_FIXEDFILEINFO is just a typedef for this struct.  In your
// client code, you would use VS_FIXEDFILEINFO instead.
struct tagVS_FIXEDFILEINFO;


namespace mvd
{
    //=================================================================================================
    // 
    // This class is designed to be a reasonably low-level wrapper around the file version info
    // API provided by MS.  There are quick shortcuts for providing the most common version information
    // (the file version, product version, etc), but everything else (copyright strings, etc) you have
    // to do yourself.
    //
    // The constructor and Open() methods are designed NOT to throw for common basic errors (like the file
    // doesn't exist, or there is no version information.  Use the operator bool() to test for validity.
    // An exception will be thrown for more serious (and very rare) errors (out-of-memory, etc).
    //
    // There are two basic sources of information in the version resource:
    //    1) the 'fixed file information'
    //    2) the translation/key/value stuff
    //
    // To access the fixed file info, just call GetFixedFileInfo().  To access the translation key/value 
    // stuff is a two step process.  First call GetTranslations(), choose what language/codepage combo 
    // you're interested in (usually the first), then call GetValue() with the selected translation.  
    // Note that the value may or may not exist.
    //
    // FINALLY - this class is fully Unicode compliant.  Note that all std::string's (in and out) are 
    //           assumed to be UTF8 strings.
    //
    //

    class FileVersion
    {
    public:
        struct Translation
        {
            unsigned short language;
            unsigned short codepage;
            Translation() : language(0), codepage(0) {}
        };

    public:
        // canonical stuff
        FileVersion(boost::filesystem::path const& filename = "");      // constructor will only throw for serious (rare) errors
        FileVersion(FileVersion const& rhs);
        FileVersion& operator=(FileVersion const& rhs);
        FileVersion(FileVersion&& rhs);
        FileVersion& operator=(FileVersion&& rhs);
        ~FileVersion();

        // extract version information and ensure validity.  Open() will throw on serious errors - but not for 
        // simple ones (like the file doesn't exist, or doesn't contain any version info).
        auto Open(boost::filesystem::path const& filename) -> bool;

        // test the FileVersion object for valid version information.
        operator bool() const;

        // access fixed file information - throws if there is no version info
        auto GetFixedFileInfo() const -> tagVS_FIXEDFILEINFO;

        // access the translation key/value information.  First get the translations, pick one (usually the first),
        // then query for the key you wish to retrieve (which may or may not exist).  Throws if there is no version info.
        auto GetTranslations()                                                  const -> std::vector<Translation>;
        auto GetValue(Translation const& translation, std::string const& key)   const -> std::string;

        // some simple shortcuts to the most common items that you would generally wish to access.  Will throw if there
        // is no valid version information.
        auto GetFileDate()           const -> unsigned long long;
        auto GetFileVersion()        const -> unsigned long long;
        auto GetProductVersion()     const -> unsigned long long;
        auto GetFileVersionString()  const -> std::string;

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
