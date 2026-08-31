#include "GameClient.h"

#include <QDebug>

GameClient::GameClient(QObject* parent)
    : QObject(parent)
    , socket_(new QTcpSocket(this))
{
    QObject::connect(socket_, &QTcpSocket::connected, this, &GameClient::onSocketConnected);
    QObject::connect(socket_, &QTcpSocket::disconnected, this, &GameClient::onSocketDisconnected);
    QObject::connect(socket_, &QTcpSocket::errorOccurred, this, &GameClient::onSocketError);
    QObject::connect(socket_, &QTcpSocket::readyRead, this, &GameClient::onReadyRead);
}
GameClient::~GameClient()
{
    disconnect();
}

void GameClient::connect(const QString& host, uint16_t port)
{
    host_ = host;
    port_ = port;
    qDebug() << "Connecting to" << host << ":" << port;
    socket_->connectToHost(host_, port_);
}
void GameClient::disconnect()
{
    if(socket_->state() != QAbstractSocket::UnconnectedState)
    {
        socket_->disconnectFromHost();
    }
}
bool GameClient::isConnected() const
{
    return socket_->state() == QAbstractSocket::ConnectedState;
}

void GameClient::sendMessage(GameProtocol::MsgType type, const BinaryWriter& payload)
{
    if (!isConnected())
    {
        qDebug() << "[SendMessage] NOT connected, dropping msg_type=" << type;
        return;
    }
    qDebug() << "[SendMessage] sending msg_type=" << type << " payload=" << payload.size();

    BinaryWriter frame;
    frame.writeU32(static_cast<uint32_t>(GameProtocol::HEADER_SIZE + payload.size()));
    frame.writeU16(static_cast<uint16_t>(type));
    frame.writeBytes(payload.ptr(), payload.size());

    socket_->write(reinterpret_cast<const char*>(frame.ptr()), frame.size());
}

void GameClient::registerHandler(GameProtocol::MsgType type, MsgHandler handler)
{
    handlers_[static_cast<uint16_t>(type)] = std::move(handler);
}

void GameClient::processBuffer()
{
    while(recvBuf_.size() >= static_cast<int>(GameProtocol::HEADER_SIZE))
    {
        BinaryReader hdr(reinterpret_cast<const uint8_t*>(recvBuf_.begin()), GameProtocol::HEADER_SIZE);
        uint32_t totalLen = 0;
        uint16_t msgType  = 0;
        hdr.readU32(totalLen);
        hdr.readU16(msgType);

        if(totalLen < GameProtocol::HEADER_SIZE || totalLen > GameProtocol::MAX_FRAME)
        {
            qDebug() << "Invalid frame length: " << totalLen << "\n";
            socket_->close();
            return;
        }
        if(recvBuf_.size() < static_cast<qsizetype>(totalLen))
        {
            return;
        }

        recvBuf_.remove(0, GameProtocol::HEADER_SIZE);

        uint32_t payloadLen = totalLen - GameProtocol::HEADER_SIZE;
        const char* payload = recvBuf_.begin();
        BinaryReader reader(reinterpret_cast<const uint8_t*>(payload), payloadLen);

        auto it = handlers_.find(msgType);
        if (it != handlers_.end())
        {
            it->second(reader);
        }

        recvBuf_.remove(0, payloadLen);
    }
}

void GameClient::onSocketConnected()
{
    qDebug() << "Socket connected to" << host_ << ":" << port_;
    emit connected();
}
void GameClient::onSocketDisconnected()
{
    qDebug() << "Socket disconnected";
    emit disconnected();
}
void GameClient::onSocketError(QAbstractSocket::SocketError err)
{
    QString msg = socket_->errorString();
    qDebug() << "Socket error:" << msg;
    emit errorOccurred(msg);
}
void GameClient::onReadyRead()
{
    recvBuf_.append(socket_->readAll());
    processBuffer();
}
