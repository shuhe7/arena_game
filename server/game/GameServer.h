#pragma once

#include "../../common/GameProtocol.h"
#include "../muduo/TcpServer.h"
#include "../muduo/EventLoop.h"
#include "../muduo/TcpConnection.h"
#include "../muduo/Buffer.h"
#include "GameTypes.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

class GameServer
{
public:
    static GameServer& instance();

    void init(const std::string& configPath);

    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);

    void sendToConnection(uint64_t connId, GameProtocol::MsgType msgType, BinaryWriter& payload);
private:
    GameServer() = default;

    std::unique_ptr<EventLoop> mainLoop_;
    std::unique_ptr<TcpServer> server_;

    std::mutex connMutex_;
    std::unordered_map<uint64_t, TcpConnectionPtr> connections_;

    int port_;
};