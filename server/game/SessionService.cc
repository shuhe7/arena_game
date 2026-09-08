#include "SessionService.h"

bool SessionService::bind(uint64_t connectionId, PlayerSession session)
{
    if(connectionId == 0 || session.userId_ == 0 || sessionsByConnection_.find(connectionId) != sessionsByConnection_.end() || connectionByUserId_.find(session.userId_) != connectionByUserId_.end())
    {
        return false;
    }

    connectionByUserId_[session.userId_] = connectionId;
    sessionsByConnection_[connectionId] = std::move(session);
    return true;
}
void SessionService::remove(uint64_t connectionId)
{
    const auto session = sessionsByConnection_.find(connectionId);
    if(session == sessionsByConnection_.end())
    {
        return;
    }

    const auto userConnection = connectionByUserId_.find(session->second.userId_);
    if(userConnection != connectionByUserId_.end() && userConnection->second == connectionId)
    {
        connectionByUserId_.erase(userConnection);
    }

    sessionsByConnection_.erase(session);
}

PlayerSession *SessionService::findByConnection(uint64_t connectionId)
{
    const auto it = sessionsByConnection_.find(connectionId);
    return it == sessionsByConnection_.end() ? nullptr : &it->second;
}
const PlayerSession *SessionService::findByConnection(uint64_t connectionId) const
{
    const auto it = sessionsByConnection_.find(connectionId);
    return it == sessionsByConnection_.end() ? nullptr : &it->second;
}

uint64_t SessionService::findConnectionByUserId(uint32_t userId) const
{
    const auto it = connectionByUserId_.find(userId);
    return it == connectionByUserId_.end() ? 0 : it->second;
}
void SessionService::markInRoom(uint32_t userId, uint64_t roomId)
{
    const uint64_t connectionId = findConnectionByUserId(userId);
    if(connectionId == 0)
    {
        return;
    }

    PlayerSession* session = findByConnection(connectionId);
    if(session != nullptr)
    {
        session->roomId_ = roomId;
    }
}