#pragma once
#include <string>

namespace mvd
{
    // these functions translate between a UTF-8 string (held in a normal std::string), to 
    // a UTF-16 string (held inside a std::wstring).  This translation is lossless, 
    // unambiguous, and reversible.
    //

    auto widen (std::string  const& utf8_string)  -> std::wstring;
    auto narrow(std::wstring const& utf16_string) -> std::string;
}
