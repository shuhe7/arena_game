#include "Logger.h"
#include "Timestamp.h"

#include <mutex>

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::start()
{
    AsyncLogging::instance().start();
}

void Logger::stop()
{
    AsyncLogging::instance().stop();
}

void Logger::log(int level, const char* msg, int len)
{
    static std::once_flag once;
    std::call_once(once, []() { AsyncLogging::instance().start(); });

    LogStream ls;
    switch (level)
    {
    case INFO:
        ls << "[INFO] ";
        break;
    case ERROR:
        ls << "[ERROR] ";
        break;
    case FATAL:
        ls << "[FATAL] ";
        break;
    case DEBUG:
        ls << "[DEBUG] ";
        break;
    default:
        ls << "[UNKNOWN] ";
        break;
    }

    ls << Timestamp::now().toFormattedString() << " : ";
    ls.append(msg, len);

    AsyncLogging::instance().append(ls.buffer().data(), ls.buffer().length());
}
