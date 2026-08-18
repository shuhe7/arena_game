#pragma once

#include "noncopyable.h"
#include "LogStream.h"
#include "Thread.h"

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const std::string& basename, int flushInterval = 3);
    ~AsyncLogging();

    static AsyncLogging& instance();

    void append(const char* logline, int len);
    void start();
    void stop();
private:
    void threadFunc();

    static const int kLargeBuffer = 4000 * 1000;
    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const std::string basename_;
    const int flushInterval_;

    std::atomic_bool running_;

    Thread thread_;

    std::mutex mutex_;
    std::condition_variable cond_;

    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
};