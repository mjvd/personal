#include "Chrono.h"

#include <windows.h>
#include <cassert>

namespace
{
    //=============================================================================================
    auto GetTicksPerSecond() -> double
    {
        static LARGE_INTEGER ticks_per_second = {0};
        if (ticks_per_second.QuadPart == 0)
        {
            QueryPerformanceFrequency(&ticks_per_second);
        }
        return static_cast<double>(ticks_per_second.QuadPart);
    }
}

//=================================================================================================
auto mvd::system_clock::now() -> time_point
{
    rep ticks;
    GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&ticks));
    return time_point(duration(ticks));
}

//=================================================================================================
auto mvd::system_clock::to_time_t(mvd::system_clock::time_point const& t) -> std::time_t
{
    return static_cast<std::time_t>(t.time_since_epoch().count() / 10000000ull - 11644473600ull);
}

//=================================================================================================
auto mvd::system_clock::from_time_t(std::time_t t) -> time_point
{
    return time_point(duration(static_cast<rep>(t) * 10000000 + 116444736000000000ull));
}

//=================================================================================================
auto mvd::system_clock::to_filetime(mvd::system_clock::time_point const& t) -> _FILETIME
{
    auto const tmp = t.time_since_epoch().count();
    return *reinterpret_cast<_FILETIME const*>(&tmp);
}

//=================================================================================================
auto mvd::system_clock::from_filetime(_FILETIME t) -> time_point
{
    return time_point(duration((static_cast<rep>(t.dwHighDateTime) << 32) | static_cast<rep>(t.dwLowDateTime)));
}

//=================================================================================================
auto mvd::high_resolution_clock::now() -> time_point
{
    static auto const frequency = GetTicksPerSecond();

    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return time_point(duration(static_cast<rep>((double)t.QuadPart / frequency * period::den / period::num)));
}

