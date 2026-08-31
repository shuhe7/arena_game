#pragma once

#include <stdint.h>

namespace GameProtocol
{
    enum MsgType : uint16_t
    {
        MSG_HEARTBEAT_REQ = 0x0001,
        MSG_HEARTBEAT_RSP = 0x0002,

        MSG_LOGIN_REQ = 0x0010,
        MSG_LOGIN_RSP = 0x0011,
        MSG_REGISTER_REQ = 0x0012,
        MSG_REGISTER_RSP = 0x0013,

        MSG_MATCH_JOIN_REQ = 0x0030,
        MSG_MATCH_JOIN_RSP = 0x0031,
    };

    constexpr uint32_t kHeaderSize = 4 + 2;
    constexpr uint32_t kMaxPayloadBytes = 8192;
    constexpr uint32_t kMaxFrameSize = 64 * 1024;
    constexpr uint32_t kMaxStringBytes = 1024;

    static const uint32_t HEADER_SIZE = kHeaderSize;
    static const uint32_t MAX_PAYLOAD = kMaxPayloadBytes;
    static const uint32_t MAX_FRAME = kMaxFrameSize;
}