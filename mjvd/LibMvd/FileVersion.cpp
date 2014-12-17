#include "FileVersion.h"
#include "utf8.h"

#include <sstream>
#include <iomanip>
#include <cassert>
#include <boost/lexical_cast.hpp>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>



//=================================================================================================
class mvd::FileVersion::Impl
{
public:
    auto IsOK() const -> bool;

public:
    fs::path                    mFileName;
    unsigned long long          mFileDate;
    unsigned long long          mFileVersion;
    unsigned long long          mProductVersion;
    std::string                 mFileVersionString;
    std::vector<unsigned char>  mVersionInfo;
};

//=================================================================================================
auto mvd::FileVersion::Impl::IsOK() const -> bool
{
    return !mVersionInfo.empty();
}

//=================================================================================================
mvd::FileVersion::FileVersion(fs::path const& filename)
    : mImpl(new Impl)
{
    Open(filename);
}

//=================================================================================================
mvd::FileVersion::FileVersion(FileVersion const& rhs)
    : mImpl(new Impl(*rhs.mImpl))
{
}

//=================================================================================================
auto mvd::FileVersion::operator=(FileVersion const& rhs) -> FileVersion&
{
    if (this != &rhs)
    {
        mImpl.reset(new Impl(*rhs.mImpl));
    }

    //std::swap(rhs.mImpl, mImpl);
    return *this;
}

//=================================================================================================
mvd::FileVersion::FileVersion(FileVersion&& rhs)
    : mImpl(std::move(rhs.mImpl))
{
}

//=================================================================================================
auto mvd::FileVersion::operator=(FileVersion&& rhs) -> FileVersion&
{
    mImpl = std::move(rhs.mImpl);
    return *this;
}

//=================================================================================================
mvd::FileVersion::~FileVersion()
{
    // required when mImpl is a std::unique_ptr<>
}

//=================================================================================================
auto mvd::FileVersion::Open(fs::path const& filename) -> bool
{
    // reset state
    mImpl->mFileName       = filename;      
    mImpl->mFileDate       = 0;
    mImpl->mFileVersion    = 0;
    mImpl->mProductVersion = 0;
    mImpl->mVersionInfo.clear();
    mImpl->mFileVersionString = "0.0.0.0";

    // is the file ok?
    if (!fs::exists(filename) || !fs::is_regular_file(filename))
    {
        return false;
    }

    // get the number of bytes required for the file version information
    DWORD const numBytes = GetFileVersionInfoSizeW(mvd::widen(filename.string()).c_str(), 0);
    if (numBytes == 0)
    {
        DWORD const error = GetLastError();
        if (error == ERROR_RESOURCE_TYPE_NOT_FOUND)
        {
            // completely normal - this file doesn't contain any version information
        }
        else
        {
            assert(false);
        }
        return false;
        //throw std::runtime_error("mvd::FileVersion::Open() - unable to get the number of bytes required to store the file version information.  GetFileVersionInfoSizeW() returned error [" + boost::lexical_cast<std::string>(error) << "]");
    }

    // now that we know the number of bytes required, we can actually read in the data
    mImpl->mVersionInfo.resize(numBytes);
    if (!GetFileVersionInfoW(mvd::widen(filename.string()).c_str(), 0, numBytes, &mImpl->mVersionInfo[0]))
    {
        mImpl->mVersionInfo.clear();
        DWORD const error = GetLastError();
        assert(false);
        return false;
        //throw std::runtime_error("mvd::FileVersion::Open() - unable to read in the version information.  GetFileVersionInfoW() returned error [" + boost::lexical_cast<std:string>(error) + "]");
    }

    // retrieve the most common information from the version for later easy access
    VS_FIXEDFILEINFO const info = GetFixedFileInfo();
    mImpl->mFileDate       = (static_cast<unsigned long long>(info.dwFileDateMS)       << 32) | static_cast<unsigned long long>(info.dwFileDateLS);
    mImpl->mFileVersion    = (static_cast<unsigned long long>(info.dwFileVersionMS)    << 32) | static_cast<unsigned long long>(info.dwFileVersionLS);
    mImpl->mProductVersion = (static_cast<unsigned long long>(info.dwProductVersionMS) << 32) | static_cast<unsigned long long>(info.dwProductVersionLS);

    // and build up our human-readable version string
    unsigned short const v1 = static_cast<unsigned short>((mImpl->mFileVersion & 0xffff000000000000) >> 48);
    unsigned short const v2 = static_cast<unsigned short>((mImpl->mFileVersion & 0x0000ffff00000000) >> 32);
    unsigned short const v3 = static_cast<unsigned short>((mImpl->mFileVersion & 0x00000000ffff0000) >> 16);
    unsigned short const v4 = static_cast<unsigned short>((mImpl->mFileVersion & 0x000000000000ffff) >>  0);

    std::ostringstream oss;
    oss << v1 << "." << v2 << "." << v3 << "." << v4;
    mImpl->mFileVersionString = oss.str();

    // done
    return true;
}

//=================================================================================================
mvd::FileVersion::operator bool() const
{
    return mImpl->IsOK();
}

//=================================================================================================
auto mvd::FileVersion::GetFixedFileInfo() const -> VS_FIXEDFILEINFO
{
    if (!mImpl->IsOK()) { throw std::runtime_error("mvd::FileVersion::GetFixedFileInfo() - no version information"); }

    VS_FIXEDFILEINFO* info = 0;
    UINT length = 0;
    if (!VerQueryValueW(reinterpret_cast<const LPVOID>(const_cast<unsigned char*>(&mImpl->mVersionInfo[0])), L"\\", reinterpret_cast<void**>(&info), &length))
    {
        DWORD const error = GetLastError();
        assert(false);
        throw std::runtime_error("mvd::FileVersion::GetFixedFileInfo() - unable to get the fixed info.  VerQueryValueW() returned error [" + boost::lexical_cast<std::string>(error) + "]");
    }
    assert(length == sizeof(VS_FIXEDFILEINFO));
    return *info;
}

//=================================================================================================
auto mvd::FileVersion::GetTranslations() const -> std::vector<Translation>
{
    if (!mImpl->IsOK()) { throw std::runtime_error("mvd::FileVersion::GetTranslations() - no version information"); }

    Translation* translations = 0;
    UINT length = 0;
    if (!VerQueryValueW(reinterpret_cast<const LPVOID>(const_cast<unsigned char*>(&mImpl->mVersionInfo[0])), L"\\VarFileInfo\\Translation", reinterpret_cast<void**>(&translations), &length))
    {
        DWORD const error = GetLastError();
        if (error != ERROR_RESOURCE_TYPE_NOT_FOUND)
        {
            assert(false);
            throw std::runtime_error("mvd::FileVersion::GetTranslations() - unable to get any translations.  VerQueryValueW() returned error [" + boost::lexical_cast<std::string>(error) + "]");
        }
    }

    std::size_t const numTranslations = length / sizeof(Translation);
    return std::vector<Translation>(translations, translations + numTranslations);
}

//=================================================================================================
auto mvd::FileVersion::GetValue(Translation const& translation, std::string const& key) const -> std::string
{
    if (!mImpl->IsOK()) { throw std::runtime_error("mvd::FileVersion::GetValue() - no version information"); }

    // build sub-block ID
    std::ostringstream oss;
    oss << "\\StringFileInfo\\" << std::setw(4) << std::setfill('0') << std::hex << translation.language << std::setw(4) << std::setfill('0') << std::hex << translation.codepage << "\\" << key;

    wchar_t* value = 0;
    UINT length = 0;
    if (!VerQueryValueW(reinterpret_cast<const LPVOID>(const_cast<unsigned char*>(&mImpl->mVersionInfo[0])), const_cast<wchar_t*>(mvd::widen(oss.str()).c_str()), reinterpret_cast<void**>(&value), &length))
    {
        DWORD const error = GetLastError();
        if (error != 0 && error != ERROR_RESOURCE_TYPE_NOT_FOUND)
        {
            assert(false);
            throw std::runtime_error("mvd::FileVersion::GetValue() - unable to retrieve the value.  VerQueryValueW() returned error [" + boost::lexical_cast<std::string>(error) + "]");
        }
    }

    return mvd::narrow(value);
}

//=================================================================================================
auto mvd::FileVersion::GetFileDate()           const -> unsigned long long { if (!mImpl->IsOK()) { throw std::runtime_error("mvd::FileVersion - no version information"); } return mImpl->mFileDate;          }
auto mvd::FileVersion::GetFileVersion()        const -> unsigned long long { if (!mImpl->IsOK()) { throw std::runtime_error("mvd::FileVersion - no version information"); } return mImpl->mFileVersion;       }
auto mvd::FileVersion::GetProductVersion()     const -> unsigned long long { if (!mImpl->IsOK()) { throw std::runtime_error("mvd::FileVersion - no version information"); } return mImpl->mProductVersion;    }
auto mvd::FileVersion::GetFileVersionString()  const -> std::string        { if (!mImpl->IsOK()) { throw std::runtime_error("mvd::FileVersion - no version information"); } return mImpl->mFileVersionString; }




