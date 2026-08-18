#pragma once

#include <string.h>
#include <stdio.h>
#include <string>

#include "noncopyable.h"
#include "LogStream.h"
#include "AsyncLogging.h"

// 日志级别
enum LogLevel
{
    INFO,   // 正常
    ERROR,  // 错误
    FATAL,  // 致命
    DEBUG,  // 调试
};

class Logger : noncopyable
{
public:
    static Logger& instance();

    void start();
    void stop();

    void setLogLevel(int level) { logLevel_ = level; }
    int logLevel() const { return logLevel_; }

    // level 前缀 + 时间戳 + 消息，交给 AsyncLogging
    void log(int level, const char* msg, int len);

private:
    Logger() = default;
    int logLevel_ = INFO;
};

#define LOG_INFO(logmsgFormat, ...) \
    do \
    { \
        char buf[1024] = {0}; \
        snprintf(buf, sizeof buf, logmsgFormat, ##__VA_ARGS__); \
        Logger::instance().log(INFO, buf, (int)strlen(buf)); \
    } while(0)

#define LOG_ERROR(logmsgFormat, ...) \
    do \
    { \
        char buf[1024] = {0}; \
        snprintf(buf, sizeof buf, logmsgFormat, ##__VA_ARGS__); \
        Logger::instance().log(ERROR, buf, (int)strlen(buf)); \
    } while(0)

#define LOG_FATAL(logmsgFormat, ...) \
    do \
    { \
        char buf[1024] = {0}; \
        snprintf(buf, sizeof buf, logmsgFormat, ##__VA_ARGS__); \
        Logger::instance().log(FATAL, buf, (int)strlen(buf)); \
        exit(-1); \
    } while(0)

#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...) \
    do \
    { \
        char buf[1024] = {0}; \
        snprintf(buf, sizeof buf, logmsgFormat, ##__VA_ARGS__); \
        Logger::instance().log(DEBUG, buf, (int)strlen(buf)); \
    } while(0)
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif
