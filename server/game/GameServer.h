#pragma once

#include "../../common/GameProtocol.h"
#include "../muduo/TcpServer.h"
#include "../muduo/EventLoop.h"
#include "../muduo/TcpConnection.h"
#include "../muduo/Buffer.h"
#include "../../common/BinaryStream.h"
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

    void handleLogin(const TcpConnectionPtr& conn, BinaryReader& reader);
    void handleRegister(const TcpConnectionPtr& conn, BinaryReader& reader);
private:
    GameServer() = default;

    void loadAccounts(const std::string& path);
    void saveAccount(const std::string& user, const std::string& pass, uint32_t id);

    bool verifyToken(const std::string& userName, const std::string& passWord, uint32_t* id);

    std::unique_ptr<EventLoop> mainLoop_;
    std::unique_ptr<TcpServer> server_;

    std::mutex connMutex_;
    std::unordered_map<uint64_t, TcpConnectionPtr> connections_;

    static std::unordered_map<std::string, std::pair<std::string, uint32_t>> sAccounts_;
    static uint32_t sNextUserId_;

    std::mutex playerMutex_;
    std::unordered_map<uint64_t, PlayerConnection> connToPlayer_;
    std::unordered_map<uint32_t, uint64_t> userToConn_;

    int port_;
};