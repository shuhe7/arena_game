#include "TimerQueue.h"
#include "EventLoop.h"
#include "Logger.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <strings.h>
#include <cassert>

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if(timerfd < 0)
    {
        LOG_FATAL("Failed in timerfd_create\n");
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec  = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    ::timerfd_settime(timerfd, 0, &newValue, &oldValue);    
}

void readTimerfd(int timerfd, Timestamp now)
{
    (void)now;
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    if(n != sizeof howmany)
    {
        LOG_ERROR("TimerQueue::handleRead() reads %zd bytes instead of 8\n", n);
    }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop)
    , timerfd_(createTimerfd())
    , timerfdChannel_(loop, timerfd_)
    , timers_()
    , activeTimers_()
    , callingExpiredTimers_(false)
    , cancelingTimers_()
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}
TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);

    for(auto& entry : timers_)
    {
        delete entry.timer_;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}
void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}
void TimerQueue::addTimerInLoop(Timer *timer)
{
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);

    if(earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}
void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();

    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    auto it = activeTimers_.find(timer);

    if(it != activeTimers_.end())
    {
        size_t n = timers_.erase(Entry(it->first, it->second));
        assert(n == 1);
        delete it->first;
        activeTimers_.erase(it);
    }
    else if(callingExpiredTimers_)
    {
        cancelingTimers_.insert(timer);
    }
}
void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for(auto& entry : expired)
    {
        entry.timer_->run();
    }
    callingExpiredTimers_ = false;

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;

    Timer bound(TimerCallback(), now, 0.0);
    Entry sentry(&bound, INT64_MAX);

    auto end = timers_.lower_bound(sentry);
    expired.assign(timers_.begin(), end);
    timers_.erase(timers_.begin(), end);

    for(const Entry& it : expired)
    {
        activeTimers_.erase(ActiveTimer(it.timer_, it.sequence_));
    }
    return expired;
}
void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now)
{
    Timestamp nextExpire;
    for(const Entry& it : expired)
    {
        ActiveTimer timer(it.timer_, it.sequence_);
        if(it.timer_->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it.timer_->restart(now);
            insert(it.timer_);
        }
        else
        {
            delete it.timer_;
        }
    }

    if(!timers_.empty())
    {
        nextExpire = timers_.begin()->timer_->expiration();
    }
    if(nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
}
bool TimerQueue::insert(Timer *timer)
{
    loop_->assertInLoopThread();
    bool earliestChanged = false;
    Timestamp when = timer->expiration();

    auto it = timers_.begin();
    if(it == timers_.end() || when < it->timer_->expiration())
    {
        earliestChanged = true;
    }

    timers_.insert(Entry(timer, timer->sequence()));
    activeTimers_.insert(ActiveTimer(timer, timer->sequence()));

    return earliestChanged;
}