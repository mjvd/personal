#include "flogger.h"

#include <iomanip>
#include <mutex>
#include <map>
#include <thread>
#include <fstream>
#include <deque>
#include <cassert>
#include <atomic>
#include <chrono>
#include <condition_variable>

namespace
{
    //=============================================================================================
    class LogRegistry    
    {
    public:
        static LogRegistry& Instance();
        std::shared_ptr<flog::Flogger> Get   (std::string const& id);
        std::shared_ptr<flog::Flogger> Create(std::string const& id, std::string const& filename);
        void Shutdown();

    private:
        LogRegistry()                               = default;
        LogRegistry(LogRegistry&&)                  = delete;
        LogRegistry(LogRegistry const&)             = delete;
        LogRegistry& operator=(LogRegistry&&)       = delete;
        LogRegistry& operator=(LogRegistry const&)  = delete;

        std::mutex mutex_;
        std::map<std::string, std::shared_ptr<flog::Flogger>> floggers_;
    };

    std::shared_ptr<flog::Flogger> LogRegistry::Get(std::string const& id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto itr = floggers_.find(id);
        return itr == floggers_.end() ? nullptr : itr->second;
    }

    std::shared_ptr<flog::Flogger> LogRegistry::Create(std::string const& id, std::string const& filename)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& tmp = floggers_[id];
        if (!tmp) { tmp = std::make_shared<flog::Flogger>(filename); }
        return tmp;
    }

    void LogRegistry::Shutdown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& flogger : floggers_)
        {
            flogger.second->Shutdown();
        }
    }

    LogRegistry& LogRegistry::Instance()
    {
        static LogRegistry instance;
        return instance;
    }

#if defined WIN32

    #define WIN32_EXTRA_LEAN
    #include <windows.h>

    //  all the usual timing functions on Windows are based on the system timer.  This system timer
    //  is updated every 10-15ms.  To get better timing precision, we can increase the frequency 
    //  of the system timer down to 1ms - giving us true 1ms accurate timestamps in the log file
    //
    //  NOTE:  there is a possibility of using very slightly more power, and also of CPU-bound 
    //         applications running ever-so-slightly slower.  I was not able to measure any
    //         slowdown in my tests...  For more information, see:
    //
    //         https://randomascii.wordpress.com/2013/07/08/windows-timer-resolution-megawatts-wasted/
    //

    #pragma comment(lib, "winmm.lib")
    MMRESULT result = timeBeginPeriod(1);

    struct OsSpecific
    {
        static unsigned long GetThreadId()
        {   
            return GetCurrentThreadId();
        }
    };

#endif
}


//=================================================================================================
std::shared_ptr<flog::Flogger> flog::Get(std::string const& id) { return LogRegistry::Instance().Get(id); }
std::shared_ptr<flog::Flogger> flog::Create(std::string const& id, std::string const& filename) { return LogRegistry::Instance().Create(id, filename); }
void flog::Shutdown() { LogRegistry::Instance().Shutdown(); }


//=================================================================================================
struct flog::Flogger::Impl
{
    Impl() { worker_ = std::make_unique<std::thread>([this]() { MainLoop(); }); }

    friend detail::LineLogger;
    void LogImpl(detail::LogMessage&& message);

    std::deque<detail::LogMessage>  messages_;
    std::atomic<LogLevel>           log_level_;
    std::ofstream                   file_;
    std::mutex                      mutex_;
    std::condition_variable         condition_;
    std::unique_ptr<std::thread>    worker_;

private:
    void MainLoop();
};

void flog::Flogger::Impl::LogImpl(detail::LogMessage&& message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_level_ < LogLevel::Shutdown)
    {
        messages_.emplace_back(std::move(message));
        condition_.notify_one();
    }
}

void flog::Flogger::Impl::MainLoop()
{
    try
    {
        while (true)
        {
            detail::LogMessage message;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                while (log_level_ != LogLevel::Shutdown && messages_.empty())
                {
                    condition_.wait(lock);
                }

                if (log_level_ == LogLevel::Shutdown) { return; }
                message = std::move(messages_.front());
                messages_.pop_front();
            }

            // jump through a few hoops to convert the raw integer 'timestamp' back into a human-readable time string
            auto const time_since_epoch     = std::chrono::system_clock::duration{ message.timestamp_ };
            auto const time_point           = std::chrono::system_clock::time_point{ time_since_epoch };
            auto const seconds              = std::chrono::system_clock::to_time_t(time_point);
            auto const fractional_seconds   = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count() % 1000; 

            auto tm = std::tm{0};
            //gmtime_s(&tm, &seconds);
            localtime_s(&tm, &seconds);
            char buffer[16];
            strftime(buffer, 16, "%T", &tm);

            // log the message
            file_ << buffer << "." << std::setw(3) << std::setfill('0') << fractional_seconds << " " << message.oss_.str() << "\n";
        }
    }
    catch (std::exception& e)
    {
        std::string const error = e.what();
        assert(false);
    }
    catch (...)
    {
        assert(false);
    }
}

//=================================================================================================
flog::Flogger::Flogger(std::string const& filename) : mImpl(std::make_unique<Impl>()) 
{
    mImpl->file_.open(filename, std::ios::trunc);
}

flog::Flogger::~Flogger() 
{ 
    Shutdown(); 
}

flog::detail::LineLogger flog::Flogger::operator()() { return detail::LineLogger(mImpl.get(), mImpl->log_level_, mImpl->log_level_ <= LogLevel::All);   }
flog::detail::LineLogger flog::Flogger::Debug()      { return detail::LineLogger(mImpl.get(), mImpl->log_level_, mImpl->log_level_ <= LogLevel::Debug); }
flog::detail::LineLogger flog::Flogger::Info()       { return detail::LineLogger(mImpl.get(), mImpl->log_level_, mImpl->log_level_ <= LogLevel::Info);  }
flog::detail::LineLogger flog::Flogger::Warn()       { return detail::LineLogger(mImpl.get(), mImpl->log_level_, mImpl->log_level_ <= LogLevel::Warn);  }
flog::detail::LineLogger flog::Flogger::Error()      { return detail::LineLogger(mImpl.get(), mImpl->log_level_, mImpl->log_level_ <= LogLevel::Error); }
flog::detail::LineLogger flog::Flogger::Fatal()      { return detail::LineLogger(mImpl.get(), mImpl->log_level_, mImpl->log_level_ <= LogLevel::Fatal); }

void flog::Flogger::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(mImpl->mutex_);
        if (mImpl->log_level_ == LogLevel::Shutdown) { return; }
        mImpl->log_level_ = LogLevel::Shutdown;
        mImpl->file_.close();
        mImpl->messages_.clear();
        mImpl->condition_.notify_one();
    }
    mImpl->worker_->join();
    mImpl->worker_.reset();
}

void flog::Flogger::SetLogLevel(LogLevel log_level) 
{ 
    std::lock_guard<std::mutex>(mImpl->mutex_);
    if (mImpl->log_level_ != LogLevel::Shutdown)
    {
        mImpl->log_level_ = log_level; 
    }
}

flog::LogLevel flog::Flogger::GetLogLevel() const 
{ 
    return mImpl->log_level_; 
}

//=================================================================================================
flog::detail::LogMessage::LogMessage(LogLevel messageLevel)
    : thread_id_(OsSpecific::GetThreadId())
    , message_level_(messageLevel)
    , timestamp_(std::chrono::system_clock::now().time_since_epoch().count())
{
}

//=================================================================================================
flog::detail::LineLogger::LineLogger(Flogger::Impl* floggerImpl, LogLevel messageLevel, bool messageEnabled)
    : flogger_impl_(floggerImpl)
    , log_message_(messageLevel)
    , message_enabled_(messageEnabled)
{
}

//=================================================================================================
flog::detail::LineLogger::~LineLogger()
{
    if (message_enabled_)
    {
        flogger_impl_->LogImpl(std::move(log_message_));
    }
}
