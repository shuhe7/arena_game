#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include <memory>

class LoginScene;
class LobbyScene;
class BattleScene;
class ResultScene;
class GameClient;

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

    std::unique_ptr<GameClient> client_;

    uint32_t userId_ = 0;
    QString  userName_;
    uint32_t elo_ = 0;
};
