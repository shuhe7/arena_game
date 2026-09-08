#pragma once

#include <cstdint>
#include <unordered_map>

struct Room
{
    uint64_t roomId_ = 0;
    uint32_t firstUserId_ = 0;
    uint32_t secondUserId_ = 0;
};

class RoomService
{
public:
    uint64_t create(uint32_t firstUserId, uint32_t secondUserId);
    const Room* find(uint64_t roomId) const;
private:
    uint64_t nextRoomId_ = 1;
    std::unordered_map<uint64_t, Room> rooms_;
};
