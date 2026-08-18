#include "AsyncLogging.h"
#include "Timestamp.h"

#include <stdio.h>
#include <assert.h>
#include <chrono>

AsyncLogging::AsyncLogging(const std::string &basename, int flushInterval)
    : basename_(basename)
    , flushInterval_(flushInterval)
    , running_(false)
    , thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging")
    , currentBuffer_(new Buffer)
    , nextBuffer_(new Buffer)
    , buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}
AsyncLogging::~AsyncLogging()
{
    if(running_)
    {
        stop();
    }
}

AsyncLogging& AsyncLogging::instance()
{
    static AsyncLogging logger("mymuduo", 3);
    return logger;
}

void AsyncLogging::append(const char *logline, int len)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if(currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline, len);
    }
    else
    {
        buffers_.push_back(std::move(currentBuffer_));

        if(nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            currentBuffer_.reset(new Buffer);
        }

        currentBuffer_->append(logline, len);
        cond_.notify_one();
    }
}
void AsyncLogging::start()
{
    running_ = true;
    thread_.start();
}
void AsyncLogging::stop()
{
    running_ = false;
    cond_.notify_all();
    thread_.join();
}
void AsyncLogging::threadFunc()
{
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    Timestamp lastFlush = Timestamp::now();

    while(running_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);

            cond_.wait_for(lock, std::chrono::seconds(flushInterval_), [this](){
                return !running_ || !buffers_.empty();
            });

            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);

            if(!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        if(buffersToWrite.empty())
        {
            continue;
        }

        Timestamp now = Timestamp::now();
        char filename[256];
        snprintf(filename, sizeof filename, "%s.log", basename_.c_str());

        FILE* fp = fopen(filename, "a");

        for(auto& buffer : buffersToWrite)
        {
            if(fp)
            {
                fwrite(buffer->data(), 1, buffer->length(), fp);
            }
            fwrite(buffer->data(), 1, buffer->length(), stdout);
        }
        if (fp)
        {
            if (Timestamp::timeDifference(now, lastFlush) >= flushInterval_)
            {
                fflush(fp);
                lastFlush = now;
            }
            fclose(fp);
        }
        fflush(stdout);

        for (auto& buffer : buffersToWrite)
        {
            buffer->reset();
            if (!newBuffer1)
            {
                newBuffer1 = std::move(buffer);
            }
            else if (!newBuffer2)
            {
                newBuffer2 = std::move(buffer);
            }
        }
        buffersToWrite.clear();

        if (!running_)
        {
            break;
        }
    }
}