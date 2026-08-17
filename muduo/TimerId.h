#pragma once

#include <cstdint>

class Timer;

class TimerId
{
public:
    TimerId(Timer* timer = nullptr, int64_t sequence = 0) : timer_(timer), sequence_(sequence) {}

    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t sequence_;
};
