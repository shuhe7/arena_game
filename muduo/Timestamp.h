#pragma once

#include <cstdint>
#include <string>

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    static Timestamp now();
    static Timestamp invalid() { return Timestamp(); }

    Timestamp addTime(double seconds) const;

    static double timeDifference(Timestamp high, Timestamp low);

    bool valid() const { return microSecondsSinceEpoch_ > 0; }
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true) const;

    bool operator<(Timestamp rhs) const { return microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch_; }
    bool operator==(Timestamp rhs) const { return microSecondsSinceEpoch_ == rhs.microSecondsSinceEpoch_; }
    bool operator!=(Timestamp rhs) const { return microSecondsSinceEpoch_ != rhs.microSecondsSinceEpoch_; }

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};
