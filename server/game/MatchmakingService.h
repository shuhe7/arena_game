#pragma once

#include "MatchQueue.h"
#include "Room.h"
#include "SessionService.h"

#include <vector>

struct MatchFoundEvent
{
    uint64_t roomId_ = 0;
    MatchPair pair_;
};

class MatchmakingService
{
public:
    MatchmakingService(RoomService& roomService, SessionService& sessionService);

    bool join(MatchTicket ticket);
    bool cancel(uint32_t userId);
    std::vector<MatchFoundEvent> tick(MatchTimePoint now);

private:
    MatchQueue matchQueue_;
    RoomService& roomService_;
    SessionService& sessionService_;
};
