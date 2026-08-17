#pragma once

#include "noncopyable.h"
#include "Timestamp.h"
#include "Channel.h"
#include "Timer.h"
#include "TimerId.h"

#include <set>
#include <vector>

class EventLoop;

class TimerQueue : noncopyable
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
    void cancel(TimerId timerId);
private:
    struct Entry
    {
        Timer* timer_;
        int64_t sequence_;

        Entry(Timer* timer, int64_t sequence) : timer_(timer), sequence_(sequence) {}

        bool operator<(const Entry& rhs) const 
        {
            if(timer_->expiration() != rhs.timer_->expiration())
            { 
                return timer_->expiration() < rhs.timer_->expiration();
            }
            return sequence_ < rhs.sequence_;
        }
    };
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);
    bool insert(Timer* timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;

    TimerList timers_;
    ActiveTimerSet activeTimers_;

    bool callingExpiredTimers_;
    ActiveTimerSet cancelingTimers_;
};