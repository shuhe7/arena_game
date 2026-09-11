#include "BattleService.h"

namespace
{
    uint16_t heroDamage(GameMessages::HeroType heroType)
    {
        switch(heroType)
        {
        case GameMessages::HeroType::kWarrior:
            return 30;
        case GameMessages::HeroType::kMage:
            return 25;
        case GameMessages::HeroType::kArcher:
            return 20;
        case GameMessages::HeroType::kNone:
            return 0;
        }

        return 0;
    }
}

bool BattleService::start(const Room& room)
{
    if(room.roomId_ == 0 || room.firstUserId_ == 0 || room.secondUserId_ == 0 || room.firstUserId_ == room.secondUserId_ ||
       heroDamage(room.firstHero_) == 0 || heroDamage(room.secondHero_) == 0 ||
       battles_.find(room.roomId_) != battles_.end())
    {
        return false;
    }

    BattleState state;
    state.roomId_ = room.roomId_;
    state.firstUserId_ = room.firstUserId_;
    state.secondUserId_ = room.secondUserId_;
    state.firstHero_ = room.firstHero_;
    state.secondHero_ = room.secondHero_;
    state.activeUserId_ = room.firstUserId_;

    battles_.emplace(state.roomId_, state);
    return true;
}
const BattleState* BattleService::find(uint64_t roomId) const
{
    const auto it = battles_.find(roomId);
    return it == battles_.end() ? nullptr : &it->second;
}
bool BattleService::remove(uint64_t roomId)
{
    return battles_.erase(roomId) != 0;
}
BattleActionResult BattleService::attack(uint64_t roomId, uint32_t userId)
{
    BattleActionResult result;

    const auto it = battles_.find(roomId);
    if(it == battles_.end())
    {
        return result;
    }

    BattleState& state = it->second;
    if(state.finished_ || state.activeUserId_ != userId)
    {
        return result;
    }

    uint16_t* targetHealth = nullptr;
    uint16_t damage = 0;
    uint32_t nextUserId = 0;

    if(userId == state.firstUserId_)
    {
        targetHealth = &state.secondHealth_;
        damage = heroDamage(state.firstHero_);
        nextUserId = state.secondUserId_;
    }
    else if(userId == state.secondUserId_)
    {
        targetHealth = &state.firstHealth_;
        damage = heroDamage(state.secondHero_);
        nextUserId = state.firstUserId_;
    }
    else
    {
        return result;
    }

    if(damage == 0)
    {
        return result;
    }

    if(*targetHealth <= damage)
    {
        *targetHealth = 0;
        state.finished_ = true;
        state.winnerUserId_ = userId;
        state.activeUserId_ = 0;
    }
    else
    {
        *targetHealth -= damage;
        state.activeUserId_ = nextUserId;
    }

    result.accepted_ = true;
    result.finished_ = state.finished_;
    result.state_ = state;
    return result;
}
