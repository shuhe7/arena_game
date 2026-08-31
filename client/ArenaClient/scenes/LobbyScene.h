#pragma once

#include <cstdint>
#include <QWidget>

class QLabel;
class QPushButton;

class LobbyScene : public QWidget
{
    Q_OBJECT
public:
    explicit LobbyScene(QWidget* parent = nullptr);

    void setPlayerInfo(uint32_t userId, const QString& userName, uint32_t elo);
signals:
    void matchRequested();
private:
    void setupUi();

    QLabel* playerNameLabel_ = nullptr;
    QLabel* playerIdLabel_ = nullptr;
    QLabel* playerEloLabel_ = nullptr;
    QPushButton* matchButton_ = nullptr;
};
