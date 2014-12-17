#pragma once

//=================================================================================================
// replacement for std::chrono.  This provides a true high-resolution clock
//=================================================================================================

#include <ctime>
#include <chrono>

// forward declaration
struct _FILETIME;

namespace mvd
{
    //=============================================================================================
    class system_clock
    {
    public:
        typedef unsigned long long                      rep;
        typedef std::ratio<1, 10000000>                 period;         // 100-ns intervals
        typedef std::chrono::duration<rep, period>      duration;
        typedef std::chrono::time_point<system_clock>   time_point;

        static bool const is_steady = false;
        static auto now()                               -> time_point;

        static auto to_time_t  (time_point const& t)    -> std::time_t;
        static auto from_time_t(std::time_t t)          -> time_point;

        static auto to_filetime  (time_point const& t)  -> _FILETIME;
        static auto from_filetime(_FILETIME t)          -> time_point;
    };

    //=============================================================================================
    class high_resolution_clock
    {
    public:
        typedef std::chrono::nanoseconds    duration;
        typedef duration::rep               rep;
        typedef duration::period            period;
        typedef std::chrono::time_point<high_resolution_clock> time_point;

        static bool const is_steady = true;
        static auto  now() -> time_point;
    };

    //=============================================================================================
    typedef high_resolution_clock monotonic_clock;
}

