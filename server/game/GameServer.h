#pragma once

#include "../../common/GameProtocol.h"
#include "../muduo/TcpServer.h"
#include "../muduo/EventLoop.h"
#include "../muduo/TcpConnection.h"
#include "../muduo/Buffer.h"
#include "../../common/BinaryStream.h"
#include "GameTypes.h"
#include "AccountRepository.h"
#include "SessionService.h"
#include "../../common/GameMessages.h"
#include "MatchQueue.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <cstdint>
#include <utility>

class GameServer
{
public:
    static GameServer& instance();

    bool init(const std::string& configPath);
    void start();
    void stop();

    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);

    void sendToConnection(uint64_t connId, GameProtocol::MsgType msgType, const BinaryWriter& payload);

    void handleLogin(const TcpConnectionPtr& conn, BinaryReader& reader);
    void handleRegister(const TcpConnectionPtr& conn, BinaryReader& reader);
private:
    GameServer() = default;

    void handleMatchJoin(const TcpConnectionPtr& conn, BinaryReader& reader);

    std::unique_ptr<EventLoop> mainLoop_;
    std::unique_ptr<TcpServer> server_;

    std::mutex connMutex_;
    std::unordered_map<uint64_t, TcpConnectionPtr> connections_;

    std::unique_ptr<AccountRepository> accountRepository_;
    SessionService sessionService_;

    MatchQueue matchQueue_;

    int port_;

    bool running_;
};