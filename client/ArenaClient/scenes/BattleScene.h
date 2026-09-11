#pragma once

#include "../../../common/GameMessages.h"

#include <QWidget>
#include <cstdint>

class QLabel;

class BattleScene : public QWidget
{
public:
    explicit BattleScene(QWidget* parent = nullptr);

    void setMatchInfo(uint64_t roomId, const QString& playerName, uint32_t playerElo, const QString& opponentName, uint32_t opponentElo);
    void setHeroInfo(GameMessages::HeroType playerHero, GameMessages::HeroType opponentHero);
private:
    void setupUi();

    QLabel* roomLabel_ = nullptr;
    QLabel* playerLabel_ = nullptr;
    QLabel* opponentLabel_ = nullptr;
    QLabel* battleStatusLabel_ = nullptr;
    QLabel* playerHeroLabel_ = nullptr;
    QLabel* opponentHeroLabel_ = nullptr;
};
