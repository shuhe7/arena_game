#include "MatchQueue.h"

#include <algorithm>
#include <limits>

namespace
{
    constexpr float kInitialEloRange = 100.0;
    constexpr float kEloRangePerSecond = 20.0;
    constexpr float kMaxEloRange = 500.0;

    uint32_t eloDifferentce(uint32_t lhs, uint32_t rhs)
    {
        return lhs >= rhs ? lhs - rhs : rhs - lhs;
    }

    float allowedEloRange(const MatchTicket &ticket, MatchTimePoint now)
    {
        const MatchClock::duration elapsed = now - ticket.joinedAt_;
        if (elapsed <= MatchClock::duration::zero())
        {
            return kInitialEloRange;
        }

        const float waitedSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(elapsed).count();
        return std::min(kInitialEloRange + waitedSeconds * kEloRangePerSecond, kMaxEloRange);
    }
}

bool MatchQueue::join(MatchTicket ticket)
{
    if(ticketsByUserId_.find(ticket.userId_) != ticketsByUserId_.end())
    {
        return false;
    }

    ticketsByElo_.emplace(ticket.elo_, ticket.userId_);
    ticketsByWaitTime_.emplace(ticket.joinedAt_, ticket.userId_);
    ticketsByUserId_.emplace(ticket.userId_, std::move(ticket));
    return true;
}
bool MatchQueue::cancel(uint32_t userId)
{
    if(!contains(userId))
    {
        return false;
    }

    removeTicket(userId);
    return true;
}

std::optional<MatchPair> MatchQueue::tryMatch(MatchTimePoint now)
{
    for(const WaitKey& waitKey : ticketsByWaitTime_)
    {
        const auto firstIt = ticketsByUserId_.find(waitKey.second);
        if(firstIt == ticketsByUserId_.end())
        {
            continue;
        }

        const MatchTicket& first = firstIt->second;
        const EloKey ownEloKey{first.elo_, first.userId_};
        const auto ownEloIt = ticketsByElo_.find(ownEloKey);

        if(ownEloIt == ticketsByElo_.end())
        {
            continue;
        }

        const float range = allowedEloRange(first, now);

        const MatchTicket* bestSecond = nullptr;
        uint32_t bestDifference = std::numeric_limits<uint32_t>::max();

        const auto consider = [&](std::set<EloKey>::const_iterator candidateIt){
            if(candidateIt == ticketsByElo_.end() || candidateIt == ownEloIt)
            {
                return;
            }

            const auto ticketIt = ticketsByUserId_.find(candidateIt->second);
            if(ticketIt == ticketsByUserId_.end())
            {
                return;
            }

            const MatchTicket& candidate = ticketIt->second;
            const uint32_t difference = eloDifferentce(first.elo_, candidate.elo_);

            if(static_cast<float>(difference) > range)
            {
                return;
            }

            if(difference < bestDifference || (difference == bestDifference && (bestSecond == nullptr || candidate.userId_ < bestSecond->userId_)))
            {
                bestDifference = difference;
                bestSecond = &candidate;
            }
        };

        if(ownEloIt != ticketsByElo_.begin())
        {
            consider(std::prev(ownEloIt));
        }

        consider(std::next(ownEloIt));

        if(bestSecond == nullptr)
        {
            continue;
        }

        MatchPair result{first, *bestSecond};

        removeTicket(result.first_.userId_);
        removeTicket(result.second_.userId_);

        return result;
    }

    return std::nullopt;
}

void MatchQueue::removeTicket(uint32_t userId)
{
    const auto ticketIt = ticketsByUserId_.find(userId);
    if(ticketIt == ticketsByUserId_.end())
    {
        return;
    }

    const MatchTicket& ticket = ticketIt->second;

    ticketsByElo_.erase(EloKey{ticket.elo_, ticket.userId_});
    ticketsByWaitTime_.erase(WaitKey{ticket.joinedAt_, ticket.userId_});
    ticketsByUserId_.erase(ticketIt);
}