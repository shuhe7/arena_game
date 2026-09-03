#include "GameServer.h"

#include "../db/ConfigMgr.h"
#include "../muduo/Logger.h"
#include "../muduo/InetAddress.h"

#include <fstream>
#include <sstream>

static const std::string kAccountFile = "accounts.txt";

namespace
{
    GameMessages::ErrorCode toErrorCode(AccountResult result)
    {
        switch (result)
        {
        case AccountResult::kOk:
            return GameMessages::ErrorCode::kNone;
        case AccountResult::kDuplicateUser:
            return GameMessages::ErrorCode::kDuplicateUserName;
        case AccountResult::kInvalidCredentials:
            return GameMessages::ErrorCode::kInvalidCredentials;
        case AccountResult::kInvalidInput:
            return GameMessages::ErrorCode::kInvalidCommand;
        case AccountResult::kStorageError:
            return GameMessages::ErrorCode::kInternalError;
        }

        return GameMessages::ErrorCode::kInternalError;
    }

    const char* accountResultMessage(AccountResult result)
    {
        switch (result)
        {
        case AccountResult::kDuplicateUser:
            return "Username already exists";
        case AccountResult::kInvalidCredentials:
            return "Invalid username or password";
        case AccountResult::kInvalidInput:
            return "Invalid username or password format";
        case AccountResult::kStorageError:
            return "Account service temporarily unavailable";
        case AccountResult::kOk:
            return "";
        }

        return "Internal server error";
    }
}

GameServer &GameServer::instance()
{
    static GameServer server;
    return server;
}

bool GameServer::init(const std::string& configPath)
{
    ConfigMgr::instance().load(configPath);
    ConfigMgr::instance().getInt("port", &port_);

    accountRepository_ = std::make_unique<AccountRepository>(kAccountFile);
    if(!accountRepository_->load())
    {
        LOG_ERROR("Failed to load account repository\n");
        return false;
    }

    LOG_INFO("GameServer initializing on port %d\n", port_);

    mainLoop_ = std::make_unique<EventLoop>();
    server_ = std::make_unique<TcpServer>(mainLoop_.get(), InetAddress(port_, "0.0.0.0"), "GameServer", TcpServer::kReusePort);

    server_->setConnectionCallback(std::bind(&GameServer::onConnection, this, std::placeholders::_1));
    server_->setMessageCallback(std::bind(&GameServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    LOG_INFO("GameServer initialized successfully\n");
    return true;
}

void GameServer::start()
{
    LOG_INFO("GameServer starting...\n");
    running_ = true;

    server_->start();
    mainLoop_->loop();
}
void GameServer::stop()
{
    running_ = false;
    LOG_INFO("GameServer stopped\n");
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
        const PlayerSession* session = sessionService_.findByConnection(conn->id());

        if(session != nullptr)
        {
            const uint32_t userId = session->userId_;
            matchQueue_.cancel(userId);
            sessionService_.remove(conn->id());
        }
            
        {
            sessionService_.remove(conn->id());
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
            case GameProtocol::MSG_LOGIN_REQ:
                handleLogin(conn, reader);
                break;
            case GameProtocol::MSG_REGISTER_REQ:
                handleRegister(conn, reader);
                break;
            case GameProtocol::MSG_MATCH_JOIN_REQ:
                handleMatchJoin(conn, reader);
                break;
            default:
                break;
        }

        buf->retrieve(payloadLen);
    }
}

void GameServer::sendToConnection(uint64_t connId, GameProtocol::MsgType msgType, const BinaryWriter &payload)
{
    TcpConnectionPtr conn;
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = connections_.find(connId);
        if(it == connections_.end()) return;
        conn = it->second;
    }

    const size_t maxPayloadSize = static_cast<size_t>(GameProtocol::MAX_FRAME - GameProtocol::HEADER_SIZE);

    if (payload.size() > maxPayloadSize)
    {
        LOG_ERROR("Outgoing payload is too large: %zu\n", payload.size());
        conn->shutdown();
        return;
    }

    BinaryWriter frame;
    frame.writeU32((uint32_t)(GameProtocol::HEADER_SIZE + payload.size()));
    frame.writeU16((uint16_t)msgType);
    frame.writeBytes(payload.ptr(), payload.size());
    conn->send(std::string(reinterpret_cast<const char*>(frame.ptr()), frame.size()));
}

void GameServer::handleLogin(const TcpConnectionPtr& conn, BinaryReader& reader)
{
    GameMessages::LoginResponse response;
    GameMessages::LoginRequest request;

    if(!GameMessages::decode(reader, request))
    {
        response.errorCode_ = GameMessages::ErrorCode::kMalformedPayload;
        response.errorMessage_ = "Malformed login request";
    }
    else
    {
        Account account;
        const AccountResult result = accountRepository_->Verify(request.userName_, request.password_, account);

        if(result != AccountResult::kOk)
        {
            response.errorCode_ = toErrorCode(result);
            response.errorMessage_ = accountResultMessage(result);
        }
        else
        {
            PlayerSession session;
            session.userId_ = account.userId_;
            session.userName_ = account.userName_;
            session.elo_ = account.elo_;

            sessionService_.bind(conn->id(), std::move(session));

            response.success_ = true;
            response.userId_ = account.userId_;
            response.elo_ = account.elo_;
            response.userName_ = account.userName_;

            LOG_INFO("Login success: uid=%u\n", account.userId_);
        }
    }

    BinaryWriter writer;
    if(!GameMessages::encode(writer, response))
    {
        LOG_ERROR("Failed to encode login response\n");
        conn->shutdown();
        return;
    }
    sendToConnection(conn->id(), GameProtocol::MSG_LOGIN_RSP, writer);
}
void GameServer::handleRegister(const TcpConnectionPtr& conn, BinaryReader& reader)
{
    GameMessages::RegisterResponse response;
    GameMessages::RegisterRequest request;

    if (!GameMessages::decode(reader, request))
    {
        response.errorCode_ = GameMessages::ErrorCode::kMalformedPayload;
        response.errorMessage_ = "Malformed register request";
    }
    else
    {
        Account account;
        const AccountResult result = accountRepository_->Register(request.userName_, request.password_, account);
        if(result != AccountResult::kOk)
        {
            response.errorCode_ = toErrorCode(result);
            response.errorMessage_ = accountResultMessage(result);
        }
        else
        {
            response.success_ = true;
            response.userId_ = account.userId_;
            response.userName_ = account.userName_;

            LOG_INFO("Registration success: uid=%u\n", account.userId_);
        }
    }

    BinaryWriter writer;
    if (!GameMessages::encode(writer, response))
    {
        LOG_ERROR("Failed to encode register response\n");
        conn->shutdown();
        return;
    }

    sendToConnection(conn->id(), GameProtocol::MSG_REGISTER_RSP, writer);
}

void GameServer::handleMatchJoin(const TcpConnectionPtr& conn, BinaryReader& reader)
{
    GameMessages::MatchJoinRequest request;
    GameMessages::MatchJoinResponse response;

    if (!GameMessages::decode(reader, request))
    {
        response.errorCode_ = GameMessages::ErrorCode::kMalformedPayload;
        response.errorMessage_ = "Malformed match join payload";
    }
    else
    {
        const PlayerSession* session = sessionService_.findByConnection(conn->id());
        if(session == nullptr)
        {
            response.errorCode_ = GameMessages::ErrorCode::kNotAuthenticated;
            response.errorMessage_ = "Login is required before matchmaking";
        }
        else if(session->roomId_ != 0)
        {
            response.errorCode_ = GameMessages::ErrorCode::kInvalidState;
            response.errorMessage_ = "Already in a room";            
        }
        else
        {
            MatchTicket ticket;
            ticket.userId_ = session->userId_;
            ticket.elo_ = session->elo_;
            ticket.joinedAt_ = MatchClock::now();

            if(!matchQueue_.join(ticket))
            {
                response.errorCode_ = GameMessages::ErrorCode::kInvalidState;
                response.errorMessage_ = "Already in matchmaking queue";
            }
            else
            {
                response.accepted_ = true;

                LOG_INFO("Player %u joined matchmaking queue (elo=%u)\n", session->userId_, session->elo_);
            }
        }
    }

    BinaryWriter writer;
    if(!GameMessages::encode(writer, response))
    {
        LOG_ERROR("Failed to encode match join response\n");
        return;        
    }

    sendToConnection(conn->id(), GameProtocol::MSG_MATCH_JOIN_RSP, writer);
}
