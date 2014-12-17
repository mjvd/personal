#include "Utf8.h"
#include <codecvt>

//=================================================================================================
auto mvd::widen(std::string const& s) -> std::wstring
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> myconv;
    return myconv.from_bytes(s);
}

//=================================================================================================
auto mvd::narrow(std::wstring const& s) -> std::string
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> myconv;
    return myconv.to_bytes(s);
}
