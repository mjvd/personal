#pragma once

#include <string>
#include <sstream>
#include <memory>

namespace flog
{
    //=============================================================================================
    //  manage the log file(s).  By convention, the main log file for any given application should 
    //  have a <blank> ID.
    class Flogger;
    std::shared_ptr<Flogger> Get(std::string const& id = "");
    std::shared_ptr<Flogger> Create(std::string const& id, std::string const& filename);
    void Shutdown();

    //=============================================================================================
    enum class LogLevel
    {
        All,
        Debug,
        Info,
        Warn,
        Error,
        Fatal,
        Shutdown,   // for internal use only
    };

    //=============================================================================================
    namespace detail { class LineLogger; }
    class Flogger
    {
    public:
        Flogger(std::string const& filename);
        ~Flogger();

        // prevent copying and assignment
        Flogger(Flogger&&)                  = delete;
        Flogger(Flogger const&)             = delete;
        Flogger& operator=(Flogger&&)       = delete;
        Flogger& operator=(Flogger const&)  = delete;

        // shut down this logger.  logging is not possible after this point.
        void Shutdown();

        // just log the message please
        detail::LineLogger operator()();

        // the following functions are for those that like to have different log levels...
        detail::LineLogger Debug();
        detail::LineLogger Info();
        detail::LineLogger Warn();
        detail::LineLogger Error();
        detail::LineLogger Fatal();

        void SetLogLevel(LogLevel x);
        LogLevel GetLogLevel() const;

    private:
        friend class detail::LineLogger;
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };


    namespace detail 
    {
        //=========================================================================================
        //  NOTE:  this class is an implementation detail.  It is not intended to be used directly.
        //         also note that this is a move-only class.  it is not copyable (due to the internal
        //         std::ostringstream member
        //
        struct LogMessage
        {
            long long           timestamp_;
            unsigned long       thread_id_;
            LogLevel            message_level_;
            std::ostringstream  oss_;

            LogMessage(LogLevel messageLevel = LogLevel::All);
            //LogMessage(LogMessage const&)               = delete;
            //LogMessage& operator=(LogMessage const&)    = delete;

            // VS2013 doesn't currently support defaulted move construction/assignment.  Have to write it ourselves...
            //
            //LogMessage(LogMessage&&)                    = default;
            //LogMessage& operator=(LogMessage&&)         = default;

            LogMessage(LogMessage&& rhs)
                : timestamp_        (std::move(rhs.timestamp_))
                , thread_id_        (std::move(rhs.thread_id_))
                , message_level_    (std::move(rhs.message_level_))
                , oss_              (std::move(rhs.oss_))
            {
            }

            LogMessage& operator=(LogMessage&& rhs)
            {
                timestamp_      = std::move(rhs.timestamp_);
                thread_id_      = std::move(rhs.thread_id_);
                message_level_  = std::move(rhs.message_level_);
                oss_            = std::move(rhs.oss_);
                return *this;
            }
        };

        //=========================================================================================
        //  NOTE:  this class is an implementation detail.  It is not intended to be used directly.
        class LineLogger
        {
        public:
            LineLogger(Flogger::Impl* floggerImpl, LogLevel messageLevel, bool messageEnabled);
            ~LineLogger();

            LineLogger(LineLogger&&)                 = delete;
            LineLogger(LineLogger const&)            = delete;
            LineLogger& operator=(LineLogger&&)      = delete;
            LineLogger& operator=(LineLogger const&) = delete;

            template <typename T>
            LineLogger& operator<<(T const& t) { if (message_enabled_) { oss_ << t; } return *this; }

        private:
            Flogger::Impl*      flogger_impl_;
            LogMessage          log_message_;
            bool                message_enabled_;
        };
    }
}
