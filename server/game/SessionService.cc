#include "SessionService.h"

void SessionService::bind(uint64_t connectionId, PlayerSession session)
{
    const auto currentSession = sessionsByConnection_.find(connectionId);
    if(currentSession != sessionsByConnection_.end())
    {
        const auto currentUser = connectionByUserId_.find(currentSession->second.userId_);
        if(currentUser != connectionByUserId_.end() && currentUser->second == connectionId)
        {
            connectionByUserId_.erase(currentUser);
        }
    }

    const auto oldConnection = connectionByUserId_.find(session.userId_);
    if(oldConnection != connectionByUserId_.end() && oldConnection->second != connectionId)
    {
        sessionsByConnection_.erase(oldConnection->second);
    }

    connectionByUserId_[session.userId_] = connectionId;
    sessionsByConnection_[connectionId] = std::move(session);
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