#pragma once
#include <memory>
#include <string>


namespace mvd
{
    class PingStats
    {
    public:
        PingStats(int  maxHistogramValue, int idealCutoff);
        ~PingStats();

        auto ToString() const -> std::string;
        auto Reset() -> void;
        auto AddSample(double rtt) -> void;
        auto AddTimeout() -> void;

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}

