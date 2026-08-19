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
    };

    #pragma pack(push, 1)
    struct FrameHeader
    {
        uint32_t total_len;
        uint16_t msg_type;
    };
    #pragma pack(pop)

    constexpr uint32_t HEADER_SIZE = sizeof(FrameHeader);
    constexpr uint32_t MAX_FRAME = 65536;
}