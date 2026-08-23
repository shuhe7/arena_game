#include "GameServer.h"

#include "../db/ConfigMgr.h"
#include "../muduo/Logger.h"
#include "../muduo/InetAddress.h"

#include <fstream>
#include <sstream>

uint32_t GameServer::sNextUserId_ = 1;
static const std::string kAccountFile = "accounts.txt";

void GameServer::loadAccounts(const std::string& path)
{
    std::fstream file(path);
    if(!file.is_open()) { return; }

    std::string line;
    while(std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string user, pass;
        uint32_t id;
        ss >> user >> pass >> id;
        if(!user.empty() && !pass.empty())
        {
            sAccounts_[user] = {pass, id};
            if (id >= sNextUserId_) { sNextUserId_ = id + 1; }
        }
    }
    LOG_INFO("Loaded %lu accounts\n", (unsigned long)sAccounts_.size());
}
void GameServer::saveAccount(const std::string& user, const std::string& pass, uint32_t id)
{
    sAccounts_[user] = {pass, id};
    std::ofstream file(kAccountFile, std::ios::app);
    if(file.is_open())
    {
        file << user << " " << pass << " " << id << "\n";
    }
}

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

    loadAccounts(kAccountFile);

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
            auto it = connToPlayer_.find(conn->id());
            if (it != connToPlayer_.end()) 
            {
                uint32_t uid = it->second.userId_;
                if (uid > 0) userToConn_.erase(uid);
                connToPlayer_.erase(it);
            }
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
            case GameProtocol::MSG_LOGIN_REQ:
                break;
            case GameProtocol::MSG_REGISTER_REQ:
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

void GameServer::handleLogin(const TcpConnectionPtr& conn, BinaryReader& reader)
{
    BinaryWriter w;

    std::string userName, passWord;
    bool getUserName = reader.readString(userName);
    bool getPassWord = reader.readString(passWord);

    uint32_t userId = 0;

    if(!getUserName || !getPassWord)
    {
        w.writeU8(0);
        w.writeString("BinaryReader read userName or passWord error");
    }
    else if(!verifyToken(userName, passWord, &userId))
    {
        w.writeU8(0);
        w.writeString("Invalid username or password");    
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(playerMutex_);
            auto old_it = userToConn_.find(userId);
            if (old_it != userToConn_.end()) 
            {
                connToPlayer_.erase(old_it->second);
                userToConn_.erase(old_it);
            }

            PlayerConnection pc;
            pc.userId_ = userId; 
            pc.userName_ = userName; 
            pc.isOnline_ = true; 
            pc.elo_ = 1000;
            connToPlayer_[conn->id()] = pc;
            userToConn_[userId] = conn->id();
        }
        w.writeU8(1);
        w.writeU32(userId);
        w.writeString(userName);
        w.writeU32(1000);
    }
    sendToConnection(conn->id(), GameProtocol::MSG_LOGIN_RSP, w);
}
void GameServer::handleRegister(const TcpConnectionPtr& conn, BinaryReader& reader)
{
    BinaryWriter w;
    std::string userName, passWord;
    bool getUserName = reader.readString(userName);
    bool getPassWord = reader.readString(passWord);

    if(!getUserName || !getPassWord)
    {
        w.writeU8(0);
        w.writeString("BinaryReader read userName or passWord error");        
    }
    else if (sAccounts_.find(userName) != sAccounts_.end()) 
    {
        w.writeU8(0);
        w.writeString("Username already exists");
    }
    else
    {
        uint32_t userId = sNextUserId_++;
        saveAccount(userName, passWord, userId);

        w.writeU8(1);
        w.writeU32(userId);
        w.writeString(userName);
    }

    sendToConnection(conn->id(), GameProtocol::MSG_REGISTER_RSP, w);
}

bool GameServer::verifyToken(const std::string& userName, const std::string& passWord, uint32_t* id)
{
    auto it = sAccounts_.find(userName);
    if(it == sAccounts_.end() || it->second.first != passWord) { return false; }
    *id = it->second.second;
    return true;
}