#pragma once

#include "GameProtocol.h"

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <limits>

class BinaryWriter 
{
public:
    BinaryWriter() { buf_.reserve(256); }

    void writeU8(uint8_t value) { buf_.push_back(value); }
    void writeU16(uint16_t value) { writeUnsigned(value); }
    void writeU32(uint32_t value) { writeUnsigned(value); }
    void writeU64(uint64_t value) { writeUnsigned(value); }

    void writeFloat(float value) 
    {
        static_assert(sizeof(float) == sizeof(uint32_t), "unsupported float representation");
        uint32_t bits = 0;
        std::memcpy(&bits, &value, sizeof(bits));
        writeU32(bits);
    }

    bool writeString(const std::string& value) 
    {
        if (value.size() > GameProtocol::kMaxStringBytes || value.size() > std::numeric_limits<uint32_t>::max()) 
        {
            return false;
        }
        writeU32(static_cast<uint32_t>(value.size()));
        writeBytes(reinterpret_cast<const uint8_t*>(value.data()), value.size());
        return true;
    }

    void writeBytes(const uint8_t* data, size_t size) 
    {
        if (size == 0) 
        {
            return;
        }
        buf_.insert(buf_.end(), data, data + size);
    }

    const std::vector<uint8_t>& data() const { return buf_; }
    const uint8_t* ptr() const { return buf_.data(); }
    size_t size() const { return buf_.size(); }
    void clear() { buf_.clear(); }

private:
    template <typename T>
    void writeUnsigned(T value) 
    {
        static_assert(std::is_unsigned<T>::value, "T must be unsigned");
        for (size_t index = 0; index < sizeof(T); ++index) 
        {
            buf_.push_back(static_cast<uint8_t>((value >> (index * 8U)) & 0xFFU));
        }
    }

    std::vector<uint8_t> buf_;
};

class BinaryReader 
{
public:
    BinaryReader(const uint8_t* data, size_t size)
        : data_(data)
        , len_(size)
        , pos_(0) {}

    bool readU8(uint8_t& value) 
    {
        if (!has(sizeof(value))) 
        {
            return false;
        }
        value = data_[pos_++];
        return true;
    }

    bool readU16(uint16_t& value) { return readUnsigned(value); }
    bool readU32(uint32_t& value) { return readUnsigned(value); }
    bool readU64(uint64_t& value) { return readUnsigned(value); }
    bool readFloat(float& value) 
    {
        uint32_t bits = 0;
        if (!readU32(bits))
        {
            return false;
        }
        std::memcpy(&value, &bits, sizeof(value));
        return true;
    }

    bool readString(std::string& value) 
    {
        uint32_t length = 0;
        if (!readU32(length) || length > GameProtocol::kMaxStringBytes || !has(length)) 
        {
            return false;
        }
        value.assign(reinterpret_cast<const char*>(data_ + pos_), length);
        pos_ += length;
        return true;
    }

    size_t pos() const { return pos_; }
    bool eof() const { return pos_ >= len_; }

private:
    bool has(size_t count) const 
    {
        return count <= len_ - pos_;
    }

    template <typename T>
    bool readUnsigned(T& value) 
    {
        static_assert(std::is_unsigned<T>::value, "T must be unsigned");
        if (!has(sizeof(T))) 
        {
            return false;
        }
        value = 0;
        for (size_t index = 0; index < sizeof(T); ++index) 
        {
            value |= static_cast<T>(data_[pos_ + index]) << (index * 8U);
        }
        pos_ += sizeof(T);
        return true;
    }

    const uint8_t* data_;
    size_t len_;
    size_t pos_;
};
