#include "Timer.h"

int64_t Timer::s_numCreated_ = 0;

Timer::Timer(TimerCallback cb, Timestamp when, double interval)
    : timerCallback_(std::move(cb))
    , expiration_(when)
    , interval_(interval)
    , repeat_(interval > 0.0)
    , sequence_(++s_numCreated_)
{}

void Timer::run() const
{
    timerCallback_();
}

void Timer::restart(Timestamp now)
{
    if(repeat_)
    {
        expiration_ = now.addTime(interval_);
    }
    else
    {
        expiration_ = Timestamp::invalid();
    }
}
