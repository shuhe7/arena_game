#pragma once

#include "../../../common/GameMessages.h"

#include <QWidget>
#include <cstdint>

class QPushButton;

class QLabel;

class BattleScene : public QWidget
{
    Q_OBJECT
public:
    explicit BattleScene(QWidget* parent = nullptr);

    void setMatchInfo(uint64_t roomId, const QString& playerName, uint32_t playerElo, const QString& opponentName, uint32_t opponentElo);
    void setHeroInfo(GameMessages::HeroType playerHero, GameMessages::HeroType opponentHero);

    void showBattleState(uint16_t playerHealth, uint16_t opponentHealth, bool playerTurn);
    void showBattleResult(bool won);
signals:
    void attackRequested();
    void returnToLobbyRequested();
private:
    void setupUi();

    QLabel* roomLabel_ = nullptr;
    QLabel* playerLabel_ = nullptr;
    QLabel* opponentLabel_ = nullptr;
    QLabel* battleStatusLabel_ = nullptr;
    QLabel* playerHeroLabel_ = nullptr;
    QLabel* opponentHeroLabel_ = nullptr;
    QLabel* playerHealthLabel_ = nullptr;
    QLabel* opponentHealthLabel_ = nullptr;
    QPushButton* attackButton_ = nullptr;
    QPushButton* returnLobbyButton_ = nullptr;
};
