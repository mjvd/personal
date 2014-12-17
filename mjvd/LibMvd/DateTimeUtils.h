#pragma once

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace mvd
{
    namespace DateTimeUtils
    {
        auto RoundUp        (boost::posix_time::ptime dt, boost::posix_time::time_duration unit) -> boost::posix_time::ptime;
        auto RoundDown      (boost::posix_time::ptime dt, boost::posix_time::time_duration unit) -> boost::posix_time::ptime;
        auto RoundNearest   (boost::posix_time::ptime dt, boost::posix_time::time_duration unit) -> boost::posix_time::ptime;
        auto Floor          (boost::posix_time::ptime dt, boost::posix_time::time_duration unit) -> boost::posix_time::ptime;
        auto Ceiling        (boost::posix_time::ptime dt, boost::posix_time::time_duration unit) -> boost::posix_time::ptime;
    }
}
