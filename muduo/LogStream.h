#pragma once

#include "noncopyable.h"

#include <string.h>
#include <string>
#include <stdio.h>
#include <stdint.h>

template<int SIZE>
class FixedBuffer
{
public:
    FixedBuffer() : cur_(data_) {}

    void append(const char* buf, size_t len)
    {
        if(static_cast<size_t>(avail()) > len)
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }

    char* current() { return cur_; }
    int avail() const { return static_cast<int>(end() - cur_); }
    void add(size_t len) { cur_ += len; }

    void reset() { cur_ = data_; }
    void bzero() { memset(data_, 0, sizeof data_); }
private:
    const char* end() const { return data_ + sizeof data_; }

    char data_[SIZE];
    char* cur_;
};

class LogStream : noncopyable
{
public:
    typedef LogStream self;
    typedef FixedBuffer<4000> Buffer;

    self& operator<<(bool v) ;
    self& operator<<(short v);
    self& operator<<(unsigned short v);
    self& operator<<(int v) ;
    self& operator<<(unsigned int v) ;
    self& operator<<(long v) ;
    self& operator<<(unsigned long v) ;
    self& operator<<(long long v) ;
    self& operator<<(unsigned long long v) ;
    self& operator<<(const void* p) ;
    self& operator<<(float v) ;
    self& operator<<(double v);

    self& operator<<(char v) ;
    self& operator<<(const char* str);
    self& operator<<(const unsigned char* str);
    self& operator<<(const std::string& v);

    void append(const char* data, int len) { buffer_.append(data, len); }
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }
private:
    static const int kMaxNumericSize = 48;

    template<typename T> 
    void formatInteger(T v);

    void formatDouble(double v);

    Buffer buffer_;
};