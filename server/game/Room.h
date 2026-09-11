#pragma once

#include "../../common/GameMessages.h"

#include <cstdint>
#include <unordered_map>

struct Room
{
    uint64_t roomId_ = 0;
    uint32_t firstUserId_ = 0;
    uint32_t secondUserId_ = 0;
    GameMessages::HeroType firstHero_ = GameMessages::HeroType::kNone;
    GameMessages::HeroType secondHero_ = GameMessages::HeroType::kNone;
};

class RoomService
{
public:
    uint64_t create(uint32_t firstUserId, uint32_t secondUserId);
    const Room* find(uint64_t roomId) const;
    bool remove(uint64_t roomId);

    bool selectHero(uint64_t roomId, uint32_t userId, GameMessages::HeroType heroType);
    bool bothHeroesSelected(uint64_t roomId) const;
private:
    uint64_t nextRoomId_ = 1;
    std::unordered_map<uint64_t, Room> rooms_;
};
