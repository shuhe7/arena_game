#include "MatchmakingService.h"

MatchmakingService::MatchmakingService(RoomService& roomService, SessionService& sessionService)
    : roomService_(roomService)
    , sessionService_(sessionService)
{}

bool MatchmakingService::join(MatchTicket ticket)
{
    return matchQueue_.join(ticket);
}
bool MatchmakingService::cancel(uint32_t userId)
{
    return matchQueue_.cancel(userId);
}
std::vector<MatchFoundEvent> MatchmakingService::tick(MatchTimePoint now)
{
    std::vector<MatchFoundEvent> events;

    while(true)
    {
        const auto pair = matchQueue_.tryMatch(now);
        if(!pair.has_value())
        {
            break;
        }

        const uint64_t firstConnectionId = sessionService_.findConnectionByUserId(pair->first_.userId_);
        const uint64_t secondConnectionId = sessionService_.findConnectionByUserId(pair->second_.userId_);

        PlayerSession* firstSession = sessionService_.findByConnection(firstConnectionId);
        PlayerSession* secondSession = sessionService_.findByConnection(secondConnectionId);

        if(firstSession == nullptr || secondSession == nullptr ||
          firstSession->roomId_ != 0 || secondSession->roomId_ != 0)
        {
            continue;
        }

        const uint64_t roomId = roomService_.create(pair->first_.userId_, pair->second_.userId_);
        if(roomId == 0)
        {
            continue;
        }

        sessionService_.markInRoom(pair->first_.userId_, roomId);
        sessionService_.markInRoom(pair->second_.userId_, roomId);
        events.push_back(MatchFoundEvent{roomId, *pair});
    }
    return events;
}
