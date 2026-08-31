#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>
#include <unordered_map>
#include <set>

typedef std::chrono::steady_clock MatchClock;
typedef MatchClock::time_point MatchTimePoint;

struct MatchTicket
{
    uint32_t userId_ = 0;
    uint32_t elo_ = 0;
    MatchTimePoint joinedAt_{};
};

struct MatchPair
{
    MatchTicket first_;
    MatchTicket second_;
};

class MatchQueue
{
public:
    bool join(MatchTicket ticket);
    bool cancel(uint32_t userId);

    std::optional<MatchPair> tryMatch(MatchTimePoint now);

    bool contains(uint32_t userId) const { return ticketsByUserId_.find(userId) != ticketsByUserId_.end(); }
    std::size_t size() const { return ticketsByUserId_.size(); }
private:
    typedef std::pair<uint32_t, uint32_t> EloKey;
    typedef std::pair<MatchTimePoint, uint32_t> WaitKey;

    void removeTicket(uint32_t userId);

    std::unordered_map<uint32_t, MatchTicket> ticketsByUserId_;
    std::set<EloKey> ticketsByElo_;
    std::set<WaitKey> ticketsByWaitTime_;
};