#include "HeroSelectScene.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

HeroSelectScene::HeroSelectScene(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void HeroSelectScene::resetSelection()
{
    selectedHero_ = GameMessages::HeroType::kNone;
    warriorButton_->setEnabled(true);
    mageButton_->setEnabled(true);
    archerButton_->setEnabled(true);
    confirmButton_->setEnabled(false);
    heroSelectStatusLabel_->setText("CHOOSE YOUR HERO");
}

void HeroSelectScene::showSelectionAccepted()
{
    warriorButton_->setEnabled(false);
    mageButton_->setEnabled(false);
    archerButton_->setEnabled(false);
    confirmButton_->setEnabled(false);
    heroSelectStatusLabel_->setText("HERO LOCKED // WAITING FOR OPPONENT");
}
void HeroSelectScene::showSelectionRejected(const QString& errorMessage)
{
    warriorButton_->setEnabled(true);
    mageButton_->setEnabled(true);
    archerButton_->setEnabled(true);
    confirmButton_->setEnabled(selectedHero_ != GameMessages::HeroType::kNone);
    heroSelectStatusLabel_->setText(QString("HERO SELECTION FAILED: %1").arg(errorMessage));
}

void HeroSelectScene::selectHero(GameMessages::HeroType heroType)
{
    selectedHero_ = heroType;
    confirmButton_->setEnabled(true);

    switch(heroType)
    {
    case GameMessages::HeroType::kWarrior:
        heroSelectStatusLabel_->setText("SELECTED // WARRIOR");
        break;
    case GameMessages::HeroType::kMage:
        heroSelectStatusLabel_->setText("SELECTED // MAGE");
        break;
    case GameMessages::HeroType::kArcher:
        heroSelectStatusLabel_->setText("SELECTED // ARCHER");
        break;
    case GameMessages::HeroType::kNone:
        break;
    }
}

void HeroSelectScene::setupUi()
{
    setObjectName("heroSelectScene");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(64, 56, 64, 56);
    layout->setSpacing(16);
    layout->setAlignment(Qt::AlignCenter);

    auto* title = new QLabel("SELECT HERO", this);
    title->setAlignment(Qt::AlignCenter);
    title->setObjectName("heroSelectTitle");

    heroSelectStatusLabel_ = new QLabel("CHOOSE YOUR HERO", this);
    heroSelectStatusLabel_->setObjectName("heroSelectStatusLabel");
    heroSelectStatusLabel_->setAlignment(Qt::AlignCenter);

    warriorButton_ = new QPushButton("WARRIOR", this);
    warriorButton_->setObjectName("warriorButton");

    mageButton_ = new QPushButton("MAGE", this);
    mageButton_->setObjectName("mageButton");

    archerButton_ = new QPushButton("ARCHER", this);
    archerButton_->setObjectName("archerButton");

    warriorButton_->setMinimumHeight(64);
    mageButton_->setMinimumHeight(64);
    archerButton_->setMinimumHeight(64);

    confirmButton_ = new QPushButton("CONFIRM HERO", this);
    confirmButton_->setObjectName("confirmButton");
    confirmButton_->setEnabled(false);

    confirmButton_->setMinimumHeight(48);

    layout->addWidget(title);
    layout->addWidget(heroSelectStatusLabel_);
    layout->addWidget(warriorButton_);
    layout->addWidget(mageButton_);
    layout->addWidget(archerButton_);
    layout->addWidget(confirmButton_);

    setStyleSheet(R"(
    #heroSelectScene {
        background: #12161f;
        color: #edf4ff;
    }

    #heroSelectTitle {
        color: #f2c94c;
        font-size: 30px;
        font-weight: 700;
    }

    #heroSelectStatusLabel {
        color: #aeb9c9;
        font-size: 13px;
        font-weight: 600;
    }

    #warriorButton, #mageButton, #archerButton {
        background: #1d2430;
        border: 1px solid #374357;
        border-radius: 10px;
        color: #edf4ff;
        font-size: 18px;
        font-weight: 700;
        padding: 14px;
    }

    #warriorButton:hover, #mageButton:hover, #archerButton:hover {
        background: #263348;
        border-color: #f2c94c;
    }

    #warriorButton:pressed, #mageButton:pressed, #archerButton:pressed {
        background: #303e55;
    }

    #warriorButton:disabled, #mageButton:disabled, #archerButton:disabled {
        background: #171c25;
        border-color: #273141;
        color: #647188;
    }

    #confirmButton {
        background: #f2c94c;
        border: none;
        border-radius: 6px;
        color: #161a22;
        font-size: 16px;
        font-weight: 700;
    }

    #confirmButton:hover {
        background: #ffe080;
    }

    #confirmButton:pressed {
        background: #d6ad2c;
    }

    #confirmButton:disabled {
        background: #4b5361;
        color: #9da7b6;
    }
)");

    connect(warriorButton_, &QPushButton::clicked, this, [this](){
        selectHero(GameMessages::HeroType::kWarrior);
    });
    connect(mageButton_, &QPushButton::clicked, this, [this](){
        selectHero(GameMessages::HeroType::kMage);
    });
    connect(archerButton_, &QPushButton::clicked, this, [this](){
        selectHero(GameMessages::HeroType::kArcher);
    });
    connect(confirmButton_, &QPushButton::clicked, this, [this](){
        if(selectedHero_ != GameMessages::HeroType::kNone)
        {
            warriorButton_->setEnabled(false);
            mageButton_->setEnabled(false);
            archerButton_->setEnabled(false);
            confirmButton_->setEnabled(false);
            heroSelectStatusLabel_->setText("HERO SELECTED // WAITING FOR SERVER");

            emit heroConfirmed(selectedHero_);
        }
    });
}
