#include "Timestamp.h"

#include <sys/time.h>
#include <stdio.h>
#include <time.h>

Timestamp::Timestamp()
    : microSecondsSinceEpoch_(0)
{}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch)
{}

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return Timestamp(static_cast<int64_t>(tv.tv_sec) * kMicroSecondsPerSecond + tv.tv_usec);
}

Timestamp Timestamp::addTime(double seconds) const
{
    int64_t delta = static_cast<int64_t>(seconds * kMicroSecondsPerSecond);
    return Timestamp(microSecondsSinceEpoch_ + delta);
}

double Timestamp::timeDifference(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / kMicroSecondsPerSecond;
}

std::string Timestamp::toString() const
{
    char buf[64] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t micros  = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof buf, "%lld.%06lld", (long long)seconds, (long long)micros);
    return buf;
}

std::string Timestamp::toFormattedString(bool showMicroseconds) const
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    localtime_r(&seconds, &tm_time);

    if (showMicroseconds)
    {
        int micros = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
        snprintf(buf, sizeof buf, "%4d/%02d/%02d %02d:%02d:%02d.%06d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, micros);
    }
    else
    {
        snprintf(buf, sizeof buf, "%4d/%02d/%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}
