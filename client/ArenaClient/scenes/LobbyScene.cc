#include "LobbyScene.h"

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>


LobbyScene::LobbyScene(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void LobbyScene::setPlayerInfo(uint32_t userId, const QString &userName, uint32_t elo)
{
    playerNameLabel_->setText(userName);
    playerIdLabel_->setText(QString("UID: %1").arg(userId));
    playerEloLabel_->setText(QString("ELO: %1").arg(elo));
}

void LobbyScene::setupUi()
{
    setObjectName("lobbyScene");

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(64, 56, 64, 56);
    rootLayout->setSpacing(20);

    auto* titleLabel = new QLabel("ARENA", this);
    titleLabel->setObjectName("lobbyTitle");

    auto* subtitleLabel = new QLabel("CONNECTED  •  READY FOR QUEUE", this);
    subtitleLabel->setObjectName("lobbySubtitle");

    auto* playerCard = new QFrame(this);
    playerCard->setObjectName("playerCard");

    auto* cardLayout = new QVBoxLayout(playerCard);
    cardLayout->setContentsMargins(28, 24, 28, 24);
    cardLayout->setSpacing(8);

    auto* profileLabel = new QLabel("PLAYER PROFILE", playerCard);
    profileLabel->setObjectName("profileTitle");

    playerNameLabel_ = new QLabel(playerCard);
    playerNameLabel_->setObjectName("playerNameLabel");

    playerIdLabel_ = new QLabel(playerCard);
    playerIdLabel_->setObjectName("playerIdLabel");

    playerEloLabel_ = new QLabel(playerCard);
    playerEloLabel_->setObjectName("playerEloLabel");

    cardLayout->addWidget(profileLabel);
    cardLayout->addWidget(playerNameLabel_);
    cardLayout->addWidget(playerIdLabel_);
    cardLayout->addWidget(playerEloLabel_);

    matchButton_ = new QPushButton("START MATCHMAKING", this);
    matchButton_->setObjectName("matchButton");
    matchButton_->setMinimumHeight(48);

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(subtitleLabel);
    rootLayout->addSpacing(16);
    rootLayout->addWidget(playerCard);
    rootLayout->addStretch();
    rootLayout->addWidget(matchButton_);

    setStyleSheet(R"(
        #lobbyScene {
            background: #12161f;
            color: #edf4ff;
        }

        #lobbyTitle {
            color: #f2c94c;
            font-size: 26px;
            font-weight: 700;
        }

        #lobbySubtitle, #profileTitle, #playerIdLabel, #playerEloLabel {
            color: #95a4b8;
            font-size: 13px;
        }

        #playerCard {
            background: #1d2430;
            border: 1px solid #374357;
            border-radius: 10px;
        }

        #playerNameLabel {
            color: #ffffff;
            font-size: 22px;
            font-weight: 700;
        }

        #matchButton {
            background: #f2c94c;
            border: none;
            border-radius: 6px;
            color: #161a22;
            font-size: 15px;
            font-weight: 700;
        }

        #matchButton:hover {
            background: #ffe080;
        }

        #matchButton:pressed {
            background: #d6ad2c;
        }
    )");

    connect(matchButton_, &QPushButton::clicked,
            this, &LobbyScene::matchRequested);
}
