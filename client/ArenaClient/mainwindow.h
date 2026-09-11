#pragma once

#include "../../common/GameMessages.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include <memory>
#include <cstdint>

class LoginScene;
class LobbyScene;
class BattleScene;
class ResultScene;
class GameClient;
class HeroSelectScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum Scene
    {
        SCENE_LOGIN,
        SCENE_LOBBY,
        SCENE_BATTLE,
        SCENE_RESULT,
        SCENE_HERO_SELECT,
    };

    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void switchTo(Scene scene);

    // Shared state
    void setUserInfo(uint32_t uid, const QString& name, uint32_t elo);

    uint32_t    userId() const { return userId_; }
    QString     username() const { return userName_; }

    GameClient* getClient() { return client_.get(); }

    void enterLobby(uint32_t userId, const QString& userName, uint32_t elo);
    void enterBattle(uint64_t roomId, const QString& opponentName, uint32_t opponentElo);
    void enterHeroSelect(uint64_t roomId, const QString& opponentUserName, uint32_t opponentElo);
    void startBattle(uint64_t roomId, GameMessages::HeroType playerHero, GameMessages::HeroType opponentHero);
signals:
    void userLoggedIn(uint32_t uid, const QString& name);

private:
    void initUi();
    void initNetwork();

    QStackedWidget* stack_ = nullptr;
    LoginScene*     loginScene_ = nullptr;
    LobbyScene*     lobbyScene_ = nullptr;
    BattleScene*    battleScene_ = nullptr;
    ResultScene*    resultScene_ = nullptr;
    HeroSelectScene* heroSelectScene_ = nullptr;

    std::unique_ptr<GameClient> client_;

    uint32_t userId_ = 0;
    QString  userName_;
    uint32_t elo_ = 0;

    uint64_t matchedRoomId_ = 0;
    QString matchedOpponentName_;
    uint32_t matchedOpponentElo_ = 0;
};
