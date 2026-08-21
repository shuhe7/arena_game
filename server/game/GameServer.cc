#include "GameServer.h"

#include "../db/ConfigMgr.h"
#include "../muduo/Logger.h"
#include "../muduo/InetAddress.h"

#include <fstream>

GameServer &GameServer::instance()
{
    static GameServer server;
    return server;
}

void GameServer::init(const std::string& configPath)
{
    ConfigMgr::instance().load(configPath);

    ConfigMgr::instance().getInt("port", &port_);

    LOG_INFO("GameServer initializing on port %d\n", port_);

    mainLoop_.reset(new EventLoop());
    server_.reset(new TcpServer(mainLoop_.get(), InetAddress(port_, "0.0.0.0"), "GameServer", TcpServer::kReusePort));

    server_->setConnectionCallback(std::bind(&GameServer::onConnection, this, std::placeholders::_1));
    server_->setMessageCallback(std::bind(&GameServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    LOG_INFO("GameServer initialized successfully\n");
}

void GameServer::onConnection(const TcpConnectionPtr &conn)
{
    if(conn->connected())
    {
        LOG_INFO("New connection: %lu\n", static_cast<unsigned long>(conn->id()));
        {
            std::lock_guard<std::mutex> lock(connMutex_);
            connections_[conn->id()] = conn;
        }
    }
    else
    {
        LOG_INFO("Connection closed: %lu\n", static_cast<unsigned long>(conn->id()));
        {
            std::lock_guard<std::mutex> lock(connMutex_);
            connections_.erase(conn->id());
        }
    }
}
void GameServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    (void)time;
    while(buf->readableBytes() >= GameProtocol::HEADER_SIZE)
    {
        BinaryReader hdr(reinterpret_cast<const uint8_t*>(buf->peek()), GameProtocol::HEADER_SIZE);
        uint32_t totalLen = 0;
        uint16_t msgType  = 0;
        hdr.readU32(totalLen);
        hdr.readU16(msgType);

        if(totalLen < GameProtocol::HEADER_SIZE || totalLen > GameProtocol::MAX_FRAME) 
        {
            LOG_ERROR("Invalid frame length: %u\n", totalLen);
            conn->shutdown();
            return;
        }
        if(buf->readableBytes() < totalLen) 
        {
            return;
        }

        buf->retrieve(GameProtocol::HEADER_SIZE);

        uint32_t payloadLen = totalLen - GameProtocol::HEADER_SIZE;
        const char* payload = buf->peek();
        BinaryReader reader(reinterpret_cast<const uint8_t*>(payload), payloadLen);

        switch(msgType)
        {
            case GameProtocol::MSG_HEARTBEAT_REQ:
                break;
            case GameProtocol::MSG_HEARTBEAT_RSP:
                break;
            case GameProtocol::MSG_LOGIN_REQ:
                break;
            case GameProtocol::MSG_LOGIN_RSP:
                break;
            case GameProtocol::MSG_REGISTER_REQ:
                break;
           case GameProtocol::MSG_REGISTER_RSP:
                break;
            default:
                break;
        }

        buf->retrieve(payloadLen);
    }
}

void GameServer::sendToConnection(uint64_t connId, GameProtocol::MsgType msgType, BinaryWriter &payload)
{
    TcpConnectionPtr conn;
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = connections_.find(connId);
        if(it == connections_.end()) return;
        conn = it->second;
    }

    BinaryWriter frame;
    frame.writeU32((uint32_t)(GameProtocol::HEADER_SIZE + payload.size()));
    frame.writeU16((uint16_t)msgType);
    frame.writeBytes(payload.ptr(), payload.size());
    conn->send(std::string(reinterpret_cast<const char*>(frame.ptr()), frame.size()));
}