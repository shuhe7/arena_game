#pragma once

#include "../../../common/GameProtocol.h"
#include "../../../common/BinaryStream.h"

#include <cstdint>
#include <functional>
#include <QObject>
#include <QTcpSocket>
#include <unordered_map>
#include <QByteArray>

class GameClient : public QObject
{
    Q_OBJECT
public:
    GameClient(QObject* parent = nullptr);
    ~GameClient();

    void connect(const QString& host, uint16_t port = 9999);
    void disconnect();
    bool isConnected() const;

    void sendMessage(GameProtocol::MsgType type, const BinaryWriter& payload);

    typedef std::function<void(BinaryReader&)> MsgHandler;
    void registerHandler(GameProtocol::MsgType type, MsgHandler handler);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& err);
    void messageReceived(uint16_t msg_type, const QByteArray& data);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError err);
    void onReadyRead();

private:
    void processBuffer();

    QTcpSocket* socket_;
    QByteArray recvBuf_;

    std::unordered_map<uint16_t, MsgHandler> handlers_;

    QString host_;
    int port_;
};
