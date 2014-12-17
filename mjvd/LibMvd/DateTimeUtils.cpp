#include "DateTimeUtils.h"
#include <boost/date_time/posix_time/posix_time.hpp>
namespace pt = boost::posix_time;

//=================================================================================================
auto mvd::DateTimeUtils::RoundUp(pt::ptime dt, pt::time_duration unit) -> pt::ptime
{
    auto const tmp = unit.total_microseconds() - (dt.time_of_day().total_microseconds() % unit.total_microseconds());
    return pt::ptime(dt.date(), pt::microseconds(dt.time_of_day().total_microseconds() + tmp));
}

//=================================================================================================
auto mvd::DateTimeUtils::RoundDown(pt::ptime dt, pt::time_duration unit) -> pt::ptime
{
    auto tmp = dt.time_of_day().total_microseconds() % unit.total_microseconds();
    if (tmp == 0) { tmp = unit.total_microseconds(); }
    return pt::ptime(dt.date(), pt::microseconds(dt.time_of_day().total_microseconds() - tmp));
}

//=================================================================================================
auto mvd::DateTimeUtils::RoundNearest(pt::ptime dt, pt::time_duration unit) -> pt::ptime
{
    auto const time_to_floor   = dt.time_of_day().total_microseconds() % unit.total_microseconds();
    auto const time_to_ceiling = unit.total_microseconds() - (dt.time_of_day().total_microseconds() % unit.total_microseconds());

    return time_to_floor < time_to_ceiling
        ? pt::ptime(dt.date(), pt::microseconds(dt.time_of_day().total_microseconds() - time_to_floor))
        : pt::ptime(dt.date(), pt::microseconds(dt.time_of_day().total_microseconds() + time_to_ceiling));
}

//=================================================================================================
auto mvd::DateTimeUtils::Floor(pt::ptime dt, pt::time_duration unit) -> pt::ptime
{
    auto const time_to_floor = dt.time_of_day().total_microseconds() % unit.total_microseconds();
    return pt::ptime(dt.date(), pt::microseconds(dt.time_of_day().total_microseconds() - time_to_floor));
}

//=================================================================================================
auto mvd::DateTimeUtils::Ceiling(pt::ptime dt, pt::time_duration unit) -> pt::ptime
{
    auto time_to_ceiling = unit.total_microseconds() - (dt.time_of_day().total_microseconds() % unit.total_microseconds());
    if (time_to_ceiling == unit.total_microseconds()) { time_to_ceiling = 0; }
    return pt::ptime(dt.date(), pt::microseconds(dt.time_of_day().total_microseconds() + time_to_ceiling));
}
