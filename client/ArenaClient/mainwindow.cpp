#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "network/GameClient.h"
#include "scenes/LoginScene.h"
#include "scenes/LobbyScene.h"

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

void MainWindow::initUi()
{
    stack_ = new QStackedWidget(this);

    loginScene_ = new LoginScene(this, stack_);
    stack_->addWidget(loginScene_);

    lobbyScene_ = new LobbyScene(stack_);
    stack_->addWidget(lobbyScene_);

    setCentralWidget(stack_);
    setWindowTitle("Arena PvP");
    resize(960, 600);

    switchTo(SCENE_LOGIN);
}

void MainWindow::initNetwork()
{
    client_ = std::make_unique<GameClient>();
}
