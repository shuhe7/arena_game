#include "Room.h"

namespace
{
    bool isSelectableHero(GameMessages::HeroType heroType)
    {
        return heroType == GameMessages::HeroType::kWarrior || heroType == GameMessages::HeroType::kMage || heroType == GameMessages::HeroType::kArcher;
    }
}

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

bool RoomService::selectHero(uint64_t roomId, uint32_t userId, GameMessages::HeroType heroType)
{
    if(!isSelectableHero(heroType))
    {
        return false;
    }

    const auto it = rooms_.find(roomId);
    if(it == rooms_.end())
    {
        return false;
    }

    Room& room = it->second;
    if(room.firstUserId_ == userId)
    {
        if(room.firstHero_ != GameMessages::HeroType::kNone)
        {
            return false;
        }

        room.firstHero_ = heroType;
        return true;
    }

    if(room.secondUserId_ == userId)
    {
        if(room.secondHero_ != GameMessages::HeroType::kNone)
        {
            return false;
        }

        room.secondHero_ = heroType;
        return true;
    }

    return false;
}
bool RoomService::bothHeroesSelected(uint64_t roomId) const
{
    const Room* room = find(roomId);
    return room != nullptr && room->firstHero_ != GameMessages::HeroType::kNone && room->secondHero_ != GameMessages::HeroType::kNone;
}
