#pragma once

#include "../../../common/GameMessages.h"

#include <QWidget>

class QLabel;
class QPushButton;

class HeroSelectScene : public QWidget
{
    Q_OBJECT
public:
    explicit HeroSelectScene(QWidget* parent = nullptr);

    void showSelectionAccepted();
    void showSelectionRejected(const QString& errorMessage);
signals:
    void heroConfirmed(GameMessages::HeroType heroType);
private:
    void setupUi();
    void selectHero(GameMessages::HeroType heroType);

    GameMessages::HeroType selectedHero_ = GameMessages::HeroType::kNone;

    QLabel* heroSelectStatusLabel_ = nullptr;
    QPushButton* warriorButton_ = nullptr;
    QPushButton* mageButton_ = nullptr;
    QPushButton* archerButton_ = nullptr;
    QPushButton* confirmButton_ = nullptr;
};
