#include "LogStream.h"

LogStream& LogStream::operator<<(bool v)
{
    buffer_.append(v ? "1": "0", 1);
    return *this;
}
LogStream& LogStream::operator<<(short v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned short v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned int v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(const void *p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(float v)
{
    return operator<<(static_cast<double>(v));
}
LogStream& LogStream::operator<<(double v)
{
    formatDouble(v);
    return *this;
}

LogStream& LogStream::operator<<(char v)
{
    buffer_.append(&v, 1);
    return *this;
}
LogStream& LogStream::operator<<(const char *str)
{
    if(str)
    {
        buffer_.append(str, strlen(str));
    }
    else
    {
        buffer_.append("(null)", 6);
    }
    return *this;
}
LogStream& LogStream::operator<<(const unsigned char *str)
{
    return operator<<(reinterpret_cast<const char*>(str));
}
LogStream& LogStream::operator<<(const std::string &v)
{
    buffer_.append(v.c_str(), v.size());
    return *this;
}

template<typename T> 
void LogStream::formatInteger(T v)
{
    if(buffer_.avail() >= kMaxNumericSize)
    {
        char buf[kMaxNumericSize];
        char* p = buf + sizeof buf;
        bool negative = v < 0;

        unsigned long long u = static_cast<unsigned long long>(v);
        if(negative)
        {
            u = 0 - u;
        }
        do
        {
            *--p = '0' + u % 10;
            u /= 10;
        } while (u > 0);
        if(negative)
        {
            *--p = '-';
        }
        buffer_.append(p, buf + sizeof buf - p);
    }
}

void LogStream::formatDouble(double v)
{
    if(buffer_.avail() > kMaxNumericSize)
    {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.6g", v);
        if(len > 0)
        {
            buffer_.add(len);
        }
    }
}
