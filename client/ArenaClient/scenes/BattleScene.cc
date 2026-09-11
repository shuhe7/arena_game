#include "BattleScene.h"

#include <QLabel>
#include <QVBoxLayout>

namespace
{
    const char* heroName(GameMessages::HeroType heroType)
    {
        switch(heroType)
        {
        case GameMessages::HeroType::kWarrior:
            return "WARRIOR";
        case GameMessages::HeroType::kMage:
            return "MAGE";
        case GameMessages::HeroType::kArcher:
            return "ARCHER";
        case GameMessages::HeroType::kNone:
            return "NONE";
        }

        return "UNKNOWN";
    }
}

BattleScene::BattleScene(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void BattleScene::setMatchInfo(uint64_t roomId, const QString& playerName, uint32_t playerElo, const QString& opponentName, uint32_t opponentElo)
{
    roomLabel_->setText(QString("ROOM #%1").arg(static_cast<qulonglong>(roomId)));
    playerLabel_->setText(QString("YOU // %1 (ELO %2)").arg(playerName).arg(playerElo));
    opponentLabel_->setText(QString("OPPONENT // %1 (ELO %2)").arg(opponentName).arg(opponentElo));
    battleStatusLabel_->setText("BATTLE READY");
}

void BattleScene::setHeroInfo(GameMessages::HeroType playerHero, GameMessages::HeroType opponentHero)
{
    playerHeroLabel_->setText(QString("YOUR HERO // %1").arg(heroName(playerHero)));
    opponentHeroLabel_->setText(QString("OPPONENT HERO // %1").arg(heroName(opponentHero)));
}

void BattleScene::setupUi()
{
    setObjectName("battleScene");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(64, 56, 64, 56);
    layout->setSpacing(20);
    layout->setAlignment(Qt::AlignCenter);

    roomLabel_ = new QLabel("ROOM #0", this);
    roomLabel_->setObjectName("roomLabel");
    roomLabel_->setAlignment(Qt::AlignCenter);

    playerLabel_ = new QLabel(this);
    playerLabel_->setObjectName("playerLabel");
    playerLabel_->setAlignment(Qt::AlignCenter);

    playerHeroLabel_ = new QLabel("YOUR HERO // NONE", this);
    playerHeroLabel_->setObjectName("playerHeroLabel");
    playerHeroLabel_->setAlignment(Qt::AlignCenter);

    opponentLabel_ = new QLabel(this);
    opponentLabel_->setObjectName("opponentLabel");
    opponentLabel_->setAlignment(Qt::AlignCenter);

    opponentHeroLabel_ = new QLabel("OPPONENT HERO // NONE", this);
    opponentHeroLabel_->setObjectName("opponentHeroLabel");
    opponentHeroLabel_->setAlignment(Qt::AlignCenter);

    battleStatusLabel_ = new QLabel("BATTLE READY", this);
    battleStatusLabel_->setObjectName("battleStatusLabel");
    battleStatusLabel_->setAlignment(Qt::AlignCenter);

    layout->addWidget(roomLabel_);
    layout->addWidget(playerLabel_);
    layout->addWidget(playerHeroLabel_);
    layout->addWidget(opponentLabel_);
    layout->addWidget(opponentHeroLabel_);
    layout->addWidget(battleStatusLabel_);

    setStyleSheet(R"(
        #battleScene {
            background: #12161f;
            color: #edf4ff;
        }

        #roomLabel {
            color: #f2c94c;
            font-size: 26px;
            font-weight: 700;
        }

        #playerLabel, #opponentLabel {
            font-size: 18px;
            font-weight: 600;
        }

        #battleStatusLabel {
            color: #44FF44;
            font-size: 16px;
            font-weight: 700;
        }

        #playerLabel, #opponentLabel, #playerHeroLabel, #opponentHeroLabel {
            font-size: 18px;
            font-weight: 600;
        }
    )");
}
