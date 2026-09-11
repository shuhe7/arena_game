#pragma once

#include "Room.h"

#include <cstdint>
#include <unordered_map>

struct BattleState
{
    uint64_t roomId_ = 0;
    uint32_t firstUserId_ = 0;
    uint32_t secondUserId_ = 0;
    GameMessages::HeroType firstHero_ = GameMessages::HeroType::kNone;
    GameMessages::HeroType secondHero_ = GameMessages::HeroType::kNone;
    uint16_t firstHealth_ = 100;
    uint16_t secondHealth_ = 100;
    uint32_t activeUserId_ = 0;
    bool finished_ = false;
    uint32_t winnerUserId_ = 0;
};

struct BattleActionResult
{
    bool accepted_ = false;
    bool finished_ = false;
    BattleState state_;
};

class BattleService
{
public:
    bool start(const Room& room);
    const BattleState* find(uint64_t roomId) const;
    BattleActionResult attack(uint64_t roomId, uint32_t userId);
private:
    std::unordered_map<uint64_t, BattleState> battles_;
};
