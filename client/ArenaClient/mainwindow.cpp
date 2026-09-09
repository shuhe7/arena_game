#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "network/GameClient.h"
#include "scenes/LoginScene.h"
#include "scenes/LobbyScene.h"
#include "../../common/GameMessages.h"
#include "scenes/BattleScene.h"

#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initNetwork();
    initUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::switchTo(Scene scene)
{
    switch (scene)
    {
    case SCENE_LOGIN:
        stack_->setCurrentWidget(loginScene_);
        break;
    case SCENE_LOBBY:
        lobbyScene_->setPlayerInfo(userId_, userName_, elo_);
        stack_->setCurrentWidget(lobbyScene_);
        break;
    case SCENE_BATTLE:
        stack_->setCurrentWidget(battleScene_);
        break;
    case SCENE_RESULT:
        break;
    }
}

void MainWindow::setUserInfo(uint32_t userId, const QString &userName, uint32_t elo)
{
    userId_ = userId;
    userName_ = userName;
    elo_ = elo;

    if(lobbyScene_ != nullptr)
    {
        lobbyScene_->setPlayerInfo(userId_, userName_, elo_);
    }
}

void MainWindow::enterLobby(uint32_t userId, const QString &userName, uint32_t elo)
{
    setUserInfo(userId, userName, elo);
    switchTo(SCENE_LOBBY);
}

void MainWindow::enterBattle(uint64_t roomId, const QString &opponentName, uint32_t opponentElo)
{
    battleScene_->setMatchInfo(roomId, userName_, elo_, opponentName, opponentElo);
    switchTo(SCENE_BATTLE);
}

void MainWindow::initUi()
{
    stack_ = new QStackedWidget(this);

    loginScene_ = new LoginScene(this, stack_);
    stack_->addWidget(loginScene_);

    lobbyScene_ = new LobbyScene(stack_);
    stack_->addWidget(lobbyScene_);

    battleScene_ = new BattleScene(stack_);
    stack_->addWidget(battleScene_);

    setCentralWidget(stack_);
    setWindowTitle("Arena PvP");
    resize(960, 600);

    connect(lobbyScene_, &LobbyScene::matchRequested, this, [this](){
        GameMessages::MatchJoinRequest request;
        BinaryWriter payload;
        if(!GameMessages::encode(payload, request))
        {
            lobbyScene_->showMatchJoinRejected("Failed to encode match request");
            return;
        }

        client_->sendMessage(GameProtocol::MSG_MATCH_JOIN_REQ, payload);
    });

    client_->registerHandler(GameProtocol::MSG_MATCH_JOIN_RSP, [this](BinaryReader& reader){
        GameMessages::MatchJoinResponse response;
        if(!GameMessages::decode(reader, response))
        {
            lobbyScene_->showMatchJoinRejected("Malformed match join response");
            return;
        }

        if(!response.accepted_)
        {
            lobbyScene_->showMatchJoinRejected(QString::fromStdString(response.errorMessage_));
            return;
        }

        lobbyScene_->showMatchmakingQueued();
    });

    client_->registerHandler(GameProtocol::MSG_MATCH_FOUND_NTF, [this](BinaryReader& reader){
        GameMessages::MatchFoundNotification notification;
        if(!GameMessages::decode(reader, notification))
        {
            return;
        }

        enterBattle(notification.roomId_,QString::fromStdString(notification.opponentUserName_),notification.opponentElo_);
    });

    switchTo(SCENE_LOGIN);
}

void MainWindow::initNetwork()
{
    client_ = std::make_unique<GameClient>();
}
