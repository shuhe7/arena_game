#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>

typedef std::function<void()> TimerCallback;

class Timer : noncopyable
{
public:
    Timer(TimerCallback cb, Timestamp when, double interval);

    void run() const;

    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }
    
    void restart(Timestamp now);

    static int64_t numCreated() { return s_numCreated_; }
private:    
    const TimerCallback timerCallback_;
    Timestamp expiration_;

    const double interval_;
    const bool repeat_;
    const int64_t sequence_;
    static int64_t s_numCreated_;
};