#include "Room.h"

uint64_t RoomService::create(uint32_t firstUserId, uint32_t secondUserId)
{
    if(firstUserId == 0 || secondUserId == 0 || firstUserId == secondUserId)
    {
        return 0;
    }
    const uint64_t roomId = nextRoomId_++;
    rooms_.emplace(roomId, Room{roomId, firstUserId, secondUserId});
    return roomId;
}
const Room* RoomService::find(uint64_t roomId) const
{
    const auto it = rooms_.find(roomId);
    return it == rooms_.end() ? nullptr : &it->second;
}
