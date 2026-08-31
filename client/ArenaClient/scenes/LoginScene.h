#pragma once

#include <QWidget>
#include <cstdint>

class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class MainWindow;

class LoginScene : public QWidget
{
    Q_OBJECT
public:
    explicit LoginScene(MainWindow* mainWindow, QWidget* parent = nullptr);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void switchToRegister();
    void switchToLogin();

private:
    enum class PendingAction : uint8_t
    {
        kNone,
        kLogin,
        kRegister,
    };

    void setupUi();
    void setupNetworkHandlers();

    void sendLogin(const QString& userName, const QString& password);
    void sendRegister(const QString& userName, const QString& password);

    MainWindow* mainWindow_ = nullptr;

    PendingAction pendingAction_ = PendingAction::kNone;
    QString pendingUserName_;
    QString pendingPassword_;

    QStackedWidget* stack_ = nullptr;

    QWidget* loginPage_ = nullptr;
    QLineEdit* loginUserName_ = nullptr;
    QLineEdit* loginPassword_ = nullptr;
    QPushButton* loginButton_ = nullptr;
    QPushButton* toRegisterButton_ = nullptr;
    QLabel* loginStatus_ = nullptr;

    QWidget* registerPage_ = nullptr;
    QLineEdit* registerUserName_ = nullptr;
    QLineEdit* registerPassword_ = nullptr;
    QLineEdit* registerPasswordConfirm_ = nullptr;
    QPushButton* registerButton_ = nullptr;
    QPushButton* toLoginButton_ = nullptr;
    QLabel* registerStatus_ = nullptr;
};
