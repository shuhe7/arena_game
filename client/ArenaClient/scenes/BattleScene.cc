#include "BattleScene.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

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

void BattleScene::showBattleState(uint16_t playerHealth, uint16_t opponentHealth, bool playerTurn)
{
    playerHealthLabel_->setText( QString("YOUR HP // %1").arg(playerHealth));
    opponentHealthLabel_->setText( QString("OPPONENT HP // %1").arg(opponentHealth));
    attackButton_->setEnabled(playerTurn);
    battleStatusLabel_->setText(playerTurn ? "YOUR TURN // ATTACK NOW" : "OPPONENT TURN // WAITING");
}

void BattleScene::showBattleResult(bool won)
{
    attackButton_->setEnabled(false);
    returnLobbyButton_->setEnabled(true);
    battleStatusLabel_->setText(won ? "VICTORY // BATTLE COMPLETE" : "DEFEAT // BATTLE COMPLETE");
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

    playerHealthLabel_ = new QLabel("YOUR HP // 0", this);
    playerHealthLabel_->setObjectName("playerHealthLabel");
    playerHealthLabel_->setAlignment(Qt::AlignCenter);

    playerHeroLabel_ = new QLabel("YOUR HERO // NONE", this);
    playerHeroLabel_->setObjectName("playerHeroLabel");
    playerHeroLabel_->setAlignment(Qt::AlignCenter);

    opponentLabel_ = new QLabel(this);
    opponentLabel_->setObjectName("opponentLabel");
    opponentLabel_->setAlignment(Qt::AlignCenter);

    opponentHealthLabel_ = new QLabel("OPPONENT HP // 0", this);
    opponentHealthLabel_->setObjectName("opponentHealthLabel");
    opponentHealthLabel_->setAlignment(Qt::AlignCenter);

    opponentHeroLabel_ = new QLabel("OPPONENT HERO // NONE", this);
    opponentHeroLabel_->setObjectName("opponentHeroLabel");
    opponentHeroLabel_->setAlignment(Qt::AlignCenter);

    battleStatusLabel_ = new QLabel("BATTLE READY", this);
    battleStatusLabel_->setObjectName("battleStatusLabel");
    battleStatusLabel_->setAlignment(Qt::AlignCenter);

    attackButton_ = new QPushButton("ATTACK", this);
    attackButton_->setObjectName("attackButton");
    attackButton_->setEnabled(false);
    attackButton_->setMinimumHeight(48);

    returnLobbyButton_ = new QPushButton("RETURN TO LOBBY", this);
    returnLobbyButton_->setObjectName("returnLobbyButton");
    returnLobbyButton_->setEnabled(false);
    returnLobbyButton_->setMinimumHeight(48);

    layout->addWidget(roomLabel_);
    layout->addWidget(playerLabel_);
    layout->addWidget(playerHeroLabel_);
    layout->addWidget(playerHealthLabel_);
    layout->addWidget(opponentLabel_);
    layout->addWidget(opponentHeroLabel_);
    layout->addWidget(opponentHealthLabel_);
    layout->addWidget(battleStatusLabel_);
    layout->addWidget(attackButton_);
    layout->addWidget(returnLobbyButton_);

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

        #playerHealthLabel, #opponentHealthLabel {
            color: #aeb9c9;
            font-size: 15px;
            font-weight: 600;
        }

        #attackButton, #returnLobbyButton {
            background: #f2c94c;
            border: none;
            border-radius: 6px;
            color: #161a22;
            font-size: 16px;
            font-weight: 700;
        }

        #attackButton:hover, #returnLobbyButton:hover {
            background: #ffe080;
        }

        #attackButton:pressed, #returnLobbyButton:pressed {
            background: #d6ad2c;
        }

        #attackButton:disabled, #returnLobbyButton:disabled {
            background: #4b5361;
            color: #9da7b6;
        }
    )");

    connect(attackButton_, &QPushButton::clicked, this, [this](){
        attackButton_->setEnabled(false);
        emit attackRequested();
    });

    connect(returnLobbyButton_, &QPushButton::clicked, this, [this](){
        returnLobbyButton_->setEnabled(false);
        emit returnToLobbyRequested();
    });
}
