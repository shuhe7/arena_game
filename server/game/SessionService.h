#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

struct PlayerSession
{
    uint32_t userId_ = 0;
    std::string userName_;
    uint32_t elo_ = 1000;
    uint64_t roomId_ = 0;
};

class SessionService
{
public:
    void bind(uint64_t connectionId, PlayerSession session);
    void remove(uint64_t connectionId);

    PlayerSession* findByConnection(uint64_t connectionId);
    const PlayerSession* findByConnection(uint64_t connectionId) const;

    uint64_t findConnectionByUserId(uint32_t userId) const;
    void markInRoom(uint32_t userId, uint64_t roomId);
private:
    std::unordered_map<uint64_t, PlayerSession> sessionsByConnection_;
    std::unordered_map<uint32_t, uint64_t> connectionByUserId_;
};