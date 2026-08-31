#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "network/GameClient.h"
#include "scenes/LoginScene.h"

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
    case SCENE_BATTLE:
    case SCENE_RESULT:
        break;
    }
}

void MainWindow::setUserInfo(uint32_t uid, const QString &name)
{

}

void MainWindow::initUi()
{
    stack_ = new QStackedWidget(this);

    loginScene_ = new LoginScene(this, stack_);
    stack_->addWidget(loginScene_);

    setCentralWidget(stack_);
    setWindowTitle("Arena PvP");
    resize(960, 600);

    switchTo(SCENE_LOGIN);
}

void MainWindow::initNetwork()
{
    client_ = std::make_unique<GameClient>();
}
