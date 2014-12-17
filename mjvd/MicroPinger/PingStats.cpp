#include "stdafx.h"
#include "PingStats.h"
#include <vector>
#include <numeric>
#include <utility>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
namespace pt = boost::posix_time;
namespace greg = boost::gregorian;


class mvd::PingStats::Impl
{
public:
    auto BuildVerticalHistogram   (std::ostream& os, std::vector<double> const& copy) -> void;
    auto BuildHorizontalHistogram (std::ostream& os, std::vector<double> const& copy) -> void;
    auto BuildHorizontalHistogram2(std::ostream& os, std::vector<double> const& copy) -> void;

public:
    int                 mMaxHistogramValue;
    int                 mIdealCutoff;
    int                 mTimeouts;
    std::vector<double> mSamples;
};

//=================================================================================================
auto mvd::PingStats::Impl::BuildVerticalHistogram(std::ostream& oss, std::vector<double> const& copy) -> void
{
    int const BIN_WIDTH = 20;
    std::map<int, int> histogram;
    for (int i=0; i<=1000; i+=BIN_WIDTH) { histogram[i] = 0; }

    for (auto d : copy)
    {
        // snap 'd' down to the next lowest multiple of 20
        int dd = static_cast<int>(d / 1000.0);
        dd -= dd % BIN_WIDTH;
        if (dd > 1000) { ++histogram[1000]; }
        else { ++histogram[dd]; }
    }

    // find the maximum value in the histogram
    int maximum = 0;
    for (auto i : histogram)
    {
        maximum = std::max(maximum, i.second);
    }
    maximum = std::max(maximum, mTimeouts);

    // build our graph
    double scale = 100.0 / maximum;
    for (auto i : histogram)
    {        
        oss << "\n    " << std::setw(4) << i.first << " | " << std::string(static_cast<int>(i.second * scale), '*');
    }
    oss << "\nTimeouts | " << std::string(static_cast<int>(mTimeouts * scale), '*')
        << "\n";
}

//=================================================================================================
auto mvd::PingStats::Impl::BuildHorizontalHistogram(std::ostream& oss, std::vector<double> const& copy) -> void
{
    // create a histogram with 201 bins
    int const MAX_BIN_NUMBER = 200;
    std::vector<int> histogram(MAX_BIN_NUMBER + 1, 0);

    // depending on whether 'precise' is true, the maximum value plotted will be either 200ms or 1000ms
    int const MAX_VALUE_MS = mMaxHistogramValue;
    int const BIN_WIDTH_MS = MAX_VALUE_MS / MAX_BIN_NUMBER;
    assert(MAX_VALUE_MS % BIN_WIDTH_MS == 0);

    // take each sample value and round downwards to the nearest bin
    for (auto d : copy)
    {
        // snap 'd' down to the next lowest multiple of BIN_WIDTH_MS
        auto dd = static_cast<int>(d / 1000.0 / static_cast<double>(BIN_WIDTH_MS));

        // update the histogram (clip 'dd' if necessary)
        ++histogram[dd > MAX_BIN_NUMBER ? MAX_BIN_NUMBER : dd];
    }

    // find the maximum value in the histogram
    int maximum = 0;
    for (auto i : histogram)
    {
        maximum = std::max(maximum, i);
    }
    maximum = std::max(maximum, mTimeouts);

    // re-scale our histogram so that maximum column value is 50
    double scale = 50.0 / maximum;
    for (auto& i : histogram)
    {
        i = static_cast<int>(i * scale);
    }
    mTimeouts = static_cast<int>(mTimeouts * scale);

    // draw graph
    oss << "\n";
    for (int i=50; i>=0; --i)
    {
        oss << "    ";
        for (auto j : histogram)
        {
            oss << (j > i ? '*' : ' ');
        }
        oss << "   " << (mTimeouts > i ? '*' : ' ') << "\n";
    }
    if (mMaxHistogramValue == 200)
    {
        oss << 
            "    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" \
            "    0         1         2         3         4         5         6         7         8         9         1         1         1         1         1         1         1         1         1         1         2   T\n" \
            "              0         0         0         0         0         0         0         0         0         0         1         2         3         4         5         6         7         8         9         0   O\n" \
            "                                                                                                        0         0         0         0         0         0         0         0         0         0         0\n\n";
    }
    else if (mMaxHistogramValue == 400)
    {
        oss << 
            "    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" \
            "    0         2         4         6         8         1         1         1         1         1         2         2         2         2         2         3         3         3         3         3         4   T\n" \
            "              0         0         0         0         0         2         4         6         8         0         2         4         6         8         0         2         4         6         8         0   O\n" \
            "                                                      0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0\n\n";
    }
    else if (mMaxHistogramValue == 600)
    {
        oss << 
            "    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" \
            "    0         3         6         9         1         1         1         2         2         2         3         3         3         3         4         4         4         5         5         5         6   T\n" \
            "              0         0         0         2         5         8         1         4         7         0         3         6         9         2         5         8         1         4         7         0   O\n" \
            "                                            0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0\n\n";
    }
    else if (mMaxHistogramValue == 800)
    {
        oss << 
            "    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" \
            "    0         4         8         1         1         2         2         2         3         3         4         4         4         5         5         6         6         6         7         7         8   T\n" \
            "              0         0         2         6         0         4         8         2         6         0         4         8         2         6         0         4         8         2         6         0   O\n" \
            "                                  0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0\n\n";
    }
    else if (mMaxHistogramValue == 1000)
    {
        oss << 
            "    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" \
            "    0         5         1         1         2         2         3         3         4         4         5         5         6         6         7         7         8         8         9         9         1   T\n" \
            "              0         0         5         0         5         0         5         0         5         0         5         0         5         0         5         0         5         0         5         0   O\n" \
            "                        0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0         0\n" \
            "                                                                                                                                                                                                            0\n\n";
    }
    else
    {
        assert(false);
    }
}

//=================================================================================================
mvd::PingStats::PingStats(int maxHistogramValue, int idealCutoff) : mImpl(new Impl) 
{ 
    assert(maxHistogramValue >= 200 && maxHistogramValue <= 1000 && maxHistogramValue % 200 == 0);
    assert(idealCutoff >= 1 && idealCutoff <= 1000);
    mImpl->mMaxHistogramValue = maxHistogramValue; 
    mImpl->mIdealCutoff = idealCutoff;
    Reset(); 
}
mvd::PingStats::~PingStats()                            {}
auto mvd::PingStats::Reset() -> void                    { mImpl->mSamples.clear(); mImpl->mSamples.reserve(64*1024); mImpl->mTimeouts = 0; }
auto mvd::PingStats::AddSample(double rtt_us) -> void   { mImpl->mSamples.push_back(rtt_us); }
auto mvd::PingStats::AddTimeout() -> void               { ++mImpl->mTimeouts; }

//=================================================================================================
auto mvd::PingStats::ToString() const -> std::string 
{
    if (mImpl->mSamples.empty()) { return "no data"; }

    std::vector<double> copy(begin(mImpl->mSamples), end(mImpl->mSamples));
    std::sort(begin(copy), end(copy));

    auto sum = std::accumulate(begin(copy), end(copy), 0.0);
    auto sumSquared = std::inner_product(begin(copy), end(copy), begin(copy), 0.0);
    auto mean = sum / copy.size();
    auto median = copy[copy.size() / 2];
    auto sd = std::sqrt(sumSquared / copy.size() - mean*mean);

    // what percentage of packets are below ideal cutoff?
    auto cutoffValue = static_cast<double>(mImpl->mIdealCutoff) * 1000.0;
    auto packetsBelowIdeal = static_cast<double>(std::count_if(mImpl->mSamples.begin(), mImpl->mSamples.end(), [cutoffValue](double d) -> bool { return d <= cutoffValue; }));
    auto packetsBelowIdealPercent = packetsBelowIdeal / static_cast<double>(mImpl->mSamples.size()) * 100.0;
        
    auto now = pt::second_clock::local_time();

    std::ostringstream oss;
    oss << "\n    date:               : " << greg::to_simple_string(now.date())
        << "\n    time:               : " << pt::to_simple_string(now.time_of_day())
        << "\n"
        << "\n    total ping attempts : " << (copy.size() + mImpl->mTimeouts)
        << "\n    timeouts            : " << mImpl->mTimeouts
        << "\n    packet loss         : " << std::fixed << std::setprecision(2) << (mImpl->mTimeouts*100.0 / (copy.size() + mImpl->mTimeouts)) << "%"
        << "\n    packets below " << std::setw(3) << mImpl->mIdealCutoff << "ms : " << std::fixed << std::setprecision(2) << packetsBelowIdealPercent << "%"
        << "\n"
        << "\n    responses received  : " << std::fixed << std::setprecision(0) << copy.size()
        << "\n    minimum             : " << copy.front()   << " microseconds"
        << "\n    maximum             : " << copy.back()    << " microseconds"
        << "\n    mean:               : " << mean           << " microseconds"
        << "\n    median:             : " << median         << " microseconds"
        << "\n    standard deviation  : " << sd             << " microseconds"
        << "\n"
        << "\n    Round-trip-time in milliseconds for [" << pt::to_simple_string(now) << "]:"
        << "\n";

    mImpl->BuildHorizontalHistogram(oss, copy);
    return oss.str();
}

