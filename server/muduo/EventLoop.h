#pragma once

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include "Timer.h"
#include "TimerId.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

class Channel;
class Poller;
class TimerQueue;

class EventLoop : noncopyable
{
public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    TimerId runAt(Timestamp time, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancel(TimerId timerId);

    void wakeup();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    void assertInLoopThread();

private:
    void handleRead();
    void doPendingFunctors();

    typedef std::vector<Channel*> ChannelList;

    std::atomic_bool looping_;
    std::atomic_bool quit_;

    const pid_t threadId_;

    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctors_;
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;
};
