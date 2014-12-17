#include "stdafx.h"
#include "Pinger.h"
#include "IcmpHeader.h"
#include "IPv4Header.h"
#include "PingStats.h"
#include <LibMvd/Chrono.h>

#include <functional>
#include <ostream>
#include <istream>
#include <iostream>

#include <boost/asio.hpp>
namespace ba = boost::asio;

#include <boost/date_time/posix_time/posix_time.hpp>
namespace pt = boost::posix_time;

namespace
{
    std::uint16_t const ICMP_IDENTIFIER = 13243;

    //=============================================================================================
    auto RoundUp(pt::ptime const& t, pt::time_duration const& interval) -> pt::ptime
    {
        auto tmp = interval.total_milliseconds() - (t.time_of_day().total_milliseconds() % interval.total_milliseconds());
        return pt::ptime(t.date(), pt::milliseconds(t.time_of_day().total_milliseconds() + tmp));
    }
}


//=================================================================================================
class mvd::Pinger::Impl
{
public:
    Impl(ba::io_service& io_service, std::string const& destination, int pingPeriod, int statsPeriod, bool verbose, bool precise, int maxHistogramValue, int idealCutoff);

private:
    auto StartSend    ()                          -> void;
    auto StartReceive ()                          -> void;
    auto HandleTimeout()                          -> void;
    auto HandleReceive(std::size_t bytesReceived) -> void;

private:
    ba::io_service&                         mIoService;
    ba::ip::icmp::resolver                  mResolver;
    ba::ip::icmp::endpoint                  mDestination;
    ba::ip::icmp::socket                    mSocket;
    ba::deadline_timer                      mTimer;
    unsigned short                          mSequenceNumber;
    ba::streambuf                           mReadBuffer;
    std::size_t                             mNumReplies;
    pt::ptime                               mSentTime;
    mvd::high_resolution_clock::time_point  mSentTime2;
    int                                     mPingPeriod;
    int                                     mStatsPeriod;
    bool                                    mVerbose;
    bool                                    mPrecise;
    pt::ptime                               mNextStatsTime;
    mvd::PingStats                          mStats;
};

//=================================================================================================
mvd::Pinger::Impl::Impl(ba::io_service& io_service, std::string const& destination, int pingPeriod, int statsPeriod, bool verbose, bool precise, int maxHistogramValue, int idealCutoff)
    : mIoService     (io_service)
    , mResolver      (io_service)
    , mSocket        (io_service, ba::ip::icmp::v4())
    , mTimer         (io_service)
    , mSequenceNumber(0)
    , mNumReplies    (0)
    , mPingPeriod    (pingPeriod)
    , mStatsPeriod   (statsPeriod)
    , mVerbose       (verbose)
    , mNextStatsTime (RoundUp(pt::second_clock::universal_time(), pt::minutes(mStatsPeriod)))
    , mStats         (maxHistogramValue, idealCutoff)
    , mPrecise       (precise)
{
    // resolve our destination
    ba::ip::icmp::resolver::query query(ba::ip::icmp::v4(), destination, "");
    mDestination = *mResolver.resolve(query);

    // and kick off the state machine
    StartSend();
    StartReceive();
}

//=================================================================================================
auto mvd::Pinger::Impl::StartSend() -> void
{
    // our message body
    static std::string const body = "abcdefghijklmnopqrstuvwabcdefghi";

    // create ICMP header for our echo request
    mvd::IcmpHeader echoRequest;
    echoRequest.Type            (mvd::IcmpHeader::MessageType::EchoRequest);
    echoRequest.Code            (0);
    echoRequest.Identifier      (ICMP_IDENTIFIER);
    echoRequest.SequenceNumber  (++mSequenceNumber);
    echoRequest.ComputeChecksum (begin(body), end(body));

    // encode our request packet
    ba::streambuf request;
    std::ostream os(&request);
    os << echoRequest << body;

    // send the request
    mSentTime = pt::microsec_clock::universal_time();
    mSentTime2 = mvd::high_resolution_clock::now();
    mSocket.send_to(request.data(), mDestination);

    // wait for a wee while, then timeout
    mNumReplies = 0;
    mTimer.expires_at(mSentTime + pt::seconds(1));
    mTimer.async_wait(std::bind(&mvd::Pinger::Impl::HandleTimeout, this));
}

//=================================================================================================
auto mvd::Pinger::Impl::HandleTimeout() -> void
{
    if (mNumReplies == 0)
    {
        std::cout << "timeout" << std::endl;
        mStats.AddTimeout();
    }

    auto now = pt::second_clock::universal_time();
    if (now >= mNextStatsTime)
    {
        std::cout << mStats.ToString() << std::endl;
        mStats.Reset();
        mNextStatsTime = RoundUp(now, pt::minutes(mStatsPeriod));
    }

    // delay before sending next echo request
    mTimer.expires_at(mSentTime + pt::milliseconds(mPingPeriod));
    mTimer.async_wait(std::bind(&mvd::Pinger::Impl::StartSend, this));
}

//=================================================================================================
auto mvd::Pinger::Impl::StartReceive() -> void
{
    // clear the buffer
    mReadBuffer.consume(mReadBuffer.size());

    // wait for the response
    mSocket.async_receive(
        mReadBuffer.prepare(64*1024),
        std::bind(&mvd::Pinger::Impl::HandleReceive, this, std::placeholders::_2));
}

//=================================================================================================
auto mvd::Pinger::Impl::HandleReceive(std::size_t bytesReceived) -> void
{
    // get the current time
    auto const now2 = mvd::high_resolution_clock::now();
    auto const now = pt::microsec_clock::universal_time();

    // the actual number of bytes received is committed to the buffer so that we can extract it using a std::istream object
    mReadBuffer.commit(bytesReceived);

    // decode the reply packet
    std::istream is(&mReadBuffer);
    mvd::IPv4Header ipv4Header;
    mvd::IcmpHeader icmpHeader;
    is >> ipv4Header >> icmpHeader;

    // unlike TCP and UDP, ICMP has no concept of port numbers.  This app will receive ALL of the ICMP packets that this 
    // machine receives.  We need to filter out the ones that don't apply to us.
    if (is                                                              &&
        icmpHeader.Type() == mvd::IcmpHeader::MessageType::EchoReply    &&
        icmpHeader.Identifier() == ICMP_IDENTIFIER                      &&
        icmpHeader.SequenceNumber() == mSequenceNumber)
    {
        // note that ICMP packets may be duplicated.  If this is the first reply then interrupt the timeout
        if (mNumReplies++ == 0)
        {
            mTimer.cancel();
        }

        // display some statistics
        auto rtt = std::chrono::duration_cast<std::chrono::microseconds>(now2 - mSentTime2);
        if (mVerbose)
        {
            //std::cout << "Reply from " << ipv4Header.SourceAddress() << ": bytes=" << (bytesReceived - ipv4Header.HeaderLength() - 8) << " time=" << (rtt.count()/1000) << "." << ((rtt.count()%1000)/100) << "ms TTL=" << static_cast<int>(ipv4Header.TimeToLive()) << " seq=" << icmpHeader.SequenceNumber() << std::endl;
            if (mPrecise)
            {
                std::cout << (rtt.count()/1000) << "." << std::setw(3) << std::setfill('0') << (rtt.count() % 1000) << std::endl;
            }
            else
            {
                std::cout << (rtt.count()/1000) << "." << ((rtt.count()%1000)/100) << std::endl;
            }
        }

        // and add this sample to our stats object
        mStats.AddSample(static_cast<double>(rtt.count()));
    }

    // and kick off the next receive
    StartReceive();
}

//=================================================================================================
mvd::Pinger::Pinger(ba::io_service& io_service, std::string const& destination, int pingPeriod, int statsPeriod, bool verbose, bool precise, int maxHistogramValue, int idealCutoff) 
    : mImpl(new Impl(io_service, destination, pingPeriod, statsPeriod, verbose, precise, maxHistogramValue, idealCutoff)) 
{
}

mvd::Pinger::~Pinger() {}

