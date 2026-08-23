#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <type_traits>


namespace endian 
{
    constexpr bool isLittle() 
    {
        #if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            return true;
        #elif defined(_WIN32)
            return true;
        #else
            return false;
        #endif
    }

    template<typename T>
    void toNative(T& value) 
    {
        if constexpr (isLittle()) 
        {
            return;
        } 
        else 
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (sizeof(T) == 2) value = __builtin_bswap16(value);
                else if constexpr (sizeof(T) == 4) value = __builtin_bswap32(value);
                else if constexpr (sizeof(T) == 8) value = __builtin_bswap64(value);
            }
            else if constexpr (std::is_floating_point_v<T>) 
            {
                static_assert(sizeof(T) == 4 || sizeof(T) == 8, "不支持 long double");
                typedef std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t> U;
                U bits;
                std::memcpy(&bits, &value, sizeof(T));
                toNative(bits);
                std::memcpy(&value, &bits, sizeof(T));
            }
        }
    }
}

class BinaryWriter 
{
public:
    BinaryWriter() { buf_.reserve(256); }
    void writeU8(uint8_t v)  { writeLE<uint8_t>(v); }
    void writeU16(uint16_t v) { writeLE<uint16_t>(v); }
    void writeU32(uint32_t v) { writeLE<uint32_t>(v); }
    void writeU64(uint64_t v) { writeLE<uint64_t>(v); }
    void writeFloat(float v) { writeLE<float>(v); }

    void writeString(const std::string& s) 
    {
        uint32_t len = static_cast<uint32_t>(s.size());
        if (len > 1024 * 1024 * 16) 
        {
            return;
        }
        writeLE(len);
        buf_.reserve(buf_.size() + len);
        buf_.insert(buf_.end(), s.begin(), s.end());
    }

    void writeBytes(const uint8_t* data, size_t len)
    {
        buf_.insert(buf_.end(), data, data + len);
    }

    const std::vector<uint8_t>& data() const { return buf_; }
    const uint8_t* ptr() const { return buf_.data(); }
    size_t size() const { return buf_.size(); }
    void clear() { buf_.clear(); }

private:
    template<typename T>
    void writeLE(T value) 
    {
        static_assert(std::is_arithmetic_v<T>, "只支持算术类型（整数和浮点数）");

        if constexpr (std::is_floating_point_v<T>) 
        {
            static_assert(sizeof(T) == 4 || sizeof(T) == 8, "不支持 long double");
            typedef std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t> U;
            U bits;
            std::memcpy(&bits, &value, sizeof(T));
            writeLE(bits);
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            buf_.push_back(value ? 1 : 0);
            return;
        }
        else
        {
            typedef std::make_unsigned_t<T> U;
            U val = static_cast<U>(value);

            buf_.reserve(buf_.size() + sizeof(T));
            for (size_t i = 0; i < sizeof(T); ++i) 
            {
                buf_.push_back(static_cast<uint8_t>((val >> (i * 8)) & 0xFF));
            }
        }


    }

    std::vector<uint8_t> buf_;
};

class BinaryReader 
{
public:
    BinaryReader(const uint8_t* data, size_t len)
        : data_(data)
        , len_(len)
        , pos_(0) 
    {}

    bool readU8(uint8_t& v)  { return readLE<uint8_t>(v); }
    bool readU16(uint16_t& v) { return readLE<uint16_t>(v); }
    bool readU32(uint32_t& v) { return readLE<uint32_t>(v); }
    bool readU64(uint64_t& v) { return readLE<uint64_t>(v); }
    bool readFloat(float& v) { return readLE<float>(v); }

    bool readString(std::string& out) 
    {
        uint32_t len = 0;
        if (!readLE(len)) 
        {
            return false;
        }
        if (len > 1024 * 1024 * 16) 
        {
            return false;
        }
        if (pos_ + len > len_) 
        {
            return false;
        }

        out.assign(reinterpret_cast<const char*>(data_ + pos_), len);
        pos_ += len;
        return true;
    }

    size_t pos() const { return pos_; }
    bool eof() const { return pos_ >= len_; }

private:
    template<typename T>
    bool readLE(T& out) 
    {
        static_assert(std::is_trivially_copyable_v<T>, "只支持可直接拷贝内存的类型");

        if (pos_ + sizeof(T) > len_) 
        {
            return false;
        }

        std::memcpy(&out, data_ + pos_, sizeof(T));
        pos_ += sizeof(T);

        endian::toNative(out);

        return true;
    }

    const uint8_t* data_;
    size_t len_;
    size_t pos_;
};