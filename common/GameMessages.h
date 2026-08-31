#pragma once

#include "BinaryStream.h"

#include <string>
#include <cstdint>

namespace GameMessages
{
    enum class ErrorCode : uint16_t 
    {
        kNone = 0,
        kMalformedPayload = 1,
        kInvalidCredentials = 2,
        kDuplicateUserName = 3,
        kNotAuthenticated = 4,
        kInvalidState = 5,
        kInvalidCommand = 6,
        kInternalError = 7,
    };

    struct LoginRequest 
    {
        std::string userName_;
        std::string password_;
    };

    struct RegisterRequest 
    {
        std::string userName_;
        std::string password_;
    };

    struct LoginResponse 
    {
        bool success_ = false;
        uint32_t userId_ = 0;
        uint32_t elo_ = 0;
        std::string userName_;
        ErrorCode errorCode_ = ErrorCode::kNone;
        std::string errorMessage_;
    };

    struct RegisterResponse 
    {
        bool success_ = false;
        uint32_t userId_ = 0;
        std::string userName_;
        ErrorCode errorCode_ = ErrorCode::kNone;
        std::string errorMessage_;
    };

    struct MatchFoundNotification
    {
        uint64_t roomId_ = 0;
        uint32_t opponentUserId_ = 0;
        std::string opponentUserName_;
        uint32_t opponentElo_ = 0;
    };

    inline bool encode(BinaryWriter& writer, const LoginRequest& value) 
    {
        return writer.writeString(value.userName_) && writer.writeString(value.password_);
    }

    inline bool decode(BinaryReader& reader, LoginRequest& value) 
    {
        return reader.readString(value.userName_) && reader.readString(value.password_) && reader.eof();
    }

    inline bool encode(BinaryWriter& writer, const RegisterRequest& value) 
    {
        return writer.writeString(value.userName_) && writer.writeString(value.password_);
    }

    inline bool decode(BinaryReader& reader, RegisterRequest& value) 
    {
        return reader.readString(value.userName_) && reader.readString(value.password_) && reader.eof();
    }

    inline bool encode(BinaryWriter& writer, const LoginResponse& value) 
    {
        writer.writeU8(value.success_ ? 1 : 0);
        if (value.success_)
        {
            writer.writeU32(value.userId_);
            writer.writeU32(value.elo_);
            return writer.writeString(value.userName_);
        }
        writer.writeU16(static_cast<uint16_t>(value.errorCode_));
        return writer.writeString(value.errorMessage_);
    }

    inline bool decode(BinaryReader& reader, LoginResponse& value) 
    {
        uint8_t success = 0;
        if (!reader.readU8(success)) 
        {
            return false;
        }
        value.success_ = success != 0;
        if (value.success_) 
        {
            return reader.readU32(value.userId_) && reader.readU32(value.elo_) && reader.readString(value.userName_) && reader.eof();
        }
        uint16_t errorCode = 0;
        if (!reader.readU16(errorCode) || !reader.readString(value.errorMessage_) || !reader.eof()) 
        {
            return false;
        }
        value.errorCode_ = static_cast<ErrorCode>(errorCode);
        return true;
    }

    inline bool encode(BinaryWriter& writer, const RegisterResponse& value) 
    {
        writer.writeU8(value.success_ ? 1 : 0);
        if (value.success_) 
        {
            writer.writeU32(value.userId_);
            return writer.writeString(value.userName_);
        }
        writer.writeU16(static_cast<uint16_t>(value.errorCode_));
        return writer.writeString(value.errorMessage_);
    }

    inline bool decode(BinaryReader& reader, RegisterResponse& value) 
    {
        uint8_t success = 0;
        if (!reader.readU8(success)) 
        {
            return false;
        }
        value.success_ = success != 0;
        if (value.success_) 
        {
            return reader.readU32(value.userId_) && reader.readString(value.userName_) && reader.eof();
        }
        uint16_t errorCode = 0;
        if (!reader.readU16(errorCode) || !reader.readString(value.errorMessage_) || !reader.eof()) 
        {
            return false;
        }
        value.errorCode_ = static_cast<ErrorCode>(errorCode);
        return true;
    }

    inline bool encode(BinaryWriter& writer, const MatchFoundNotification& value)
    {
        writer.writeU64(value.roomId_);
        writer.writeU32(value.opponentUserId_);

        if (!writer.writeString(value.opponentUserName_))
        {
            return false;
        }

        writer.writeU32(value.opponentElo_);
        return true;
    }

    inline bool decode(BinaryReader& reader, MatchFoundNotification& value)
    {
        return reader.readU64(value.roomId_) && reader.readU32(value.opponentUserId_) && reader.readString(value.opponentUserName_) && reader.readU32(value.opponentElo_) && reader.eof();
    } 
}