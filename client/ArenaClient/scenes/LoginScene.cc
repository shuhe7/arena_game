#include "LoginScene.h"
#include "../mainwindow.h"
#include "../network/GameClient.h"
#include "../../common/GameMessages.h"

#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QTimer>

LoginScene::LoginScene(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , mainWindow_(mainWindow)
{
    setupUi();
    setupNetworkHandlers();
}

void LoginScene::onLoginClicked()
{
    const QString userName = loginUserName_->text().trimmed();
    const QString password = loginPassword_->text();

    if(userName.isEmpty() || password.isEmpty())
    {
        loginStatus_->setText("Please fill in username and password");
        return;
    }

    GameClient* client = mainWindow_->getClient();
    if(client->isConnected())
    {
        sendLogin(userName, password);
        return;
    }

    if(pendingAction_ != PendingAction::kNone)
    {
        loginStatus_->setText("Connecting...");
        return;
    }

    pendingAction_ = PendingAction::kLogin;
    pendingUserName_ = userName;
    pendingPassword_ = password;

    loginStatus_->setText("Conncting...");
    client->connect("192.168.1.90", 9999);
}

void LoginScene::onRegisterClicked()
{
    const QString userName = registerUserName_->text().trimmed();
    const QString password = registerPassword_->text();
    const QString passwordConfirm = registerPasswordConfirm_->text();

    if (userName.isEmpty() || password.isEmpty())
    {
        registerStatus_->setText("Please fill in username and password");
        return;
    }

    if (password != passwordConfirm)
    {
        registerStatus_->setText("Passwords do not match");
        return;
    }

    GameClient* client = mainWindow_->getClient();
    if(client->isConnected())
    {
        sendRegister(userName, password);
        return;
    }

    if(pendingAction_ != PendingAction::kNone)
    {
        registerStatus_->setText("Connecting...");
        return;
    }

    pendingAction_ = PendingAction::kRegister;
    pendingUserName_ = userName;
    pendingPassword_ = password;

    registerStatus_->setText("Connecting...");
    client->connect("192.168.1.90", 9999);
}

void LoginScene::switchToRegister()
{
    loginStatus_->clear();
    registerStatus_->clear();
    stack_->setCurrentWidget(registerPage_);
}

void LoginScene::switchToLogin()
{
    loginStatus_->clear();
    registerStatus_->clear();
    stack_->setCurrentWidget(loginPage_);
}

void LoginScene::setupUi()
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(40, 40, 40, 40);
    outerLayout->setAlignment(Qt::AlignCenter);

    loginPage_ = new QWidget;
    auto* loginLayout = new QVBoxLayout(loginPage_);
    loginLayout->setAlignment(Qt::AlignCenter);

    auto* title = new QLabel("ARENA PvP");
    QFont titleFont;
    titleFont.setPointSize(32);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #FF6B35; margin-bottom: 20px;");
    loginLayout->addWidget(title);

    auto* loginGroup = new QGroupBox("Login");
    loginGroup->setFixedWidth(360);
    auto* loginForm = new QVBoxLayout(loginGroup);

    loginUserName_ = new QLineEdit;
    loginUserName_->setPlaceholderText("Username");
    loginUserName_->setMinimumHeight(36);
    loginForm->addWidget(loginUserName_);

    loginPassword_ = new QLineEdit;
    loginPassword_->setPlaceholderText("Password");
    loginPassword_->setEchoMode(QLineEdit::Password);
    loginPassword_->setMinimumHeight(36);
    loginForm->addWidget(loginPassword_);

    loginButton_ = new QPushButton("Sign In");
    loginButton_->setMinimumHeight(40);
    loginButton_->setStyleSheet("QPushButton { background: #FF6B35; color: white; " "font-weight: bold; border-radius: 4px; }" "QPushButton:hover { background: #FF8C5A; }");
    loginForm->addWidget(loginButton_);

    toRegisterButton_ = new QPushButton("Create Account");
    toRegisterButton_->setStyleSheet(
        "QPushButton { color: #AAAAAA; border: none; }");
    loginForm->addWidget(toRegisterButton_);

    loginStatus_ = new QLabel;
    loginStatus_->setAlignment(Qt::AlignCenter);
    loginStatus_->setWordWrap(true);
    loginStatus_->setStyleSheet("color: #FF4444;");
    loginForm->addWidget(loginStatus_);

    loginLayout->addWidget(loginGroup);

    registerPage_ = new QWidget;
    auto* registerLayout = new QVBoxLayout(registerPage_);
    registerLayout->setAlignment(Qt::AlignCenter);

    auto* registerTitle = new QLabel("Create Account");
    QFont registerTitleFont;
    registerTitleFont.setPointSize(24);
    registerTitleFont.setBold(true);
    registerTitle->setFont(registerTitleFont);
    registerTitle->setAlignment(Qt::AlignCenter);
    registerTitle->setStyleSheet("color: #FF6B35; margin-bottom: 20px;");
    registerLayout->addWidget(registerTitle);

    auto* registerGroup = new QGroupBox("Register");
    registerGroup->setFixedWidth(360);
    auto* registerForm = new QVBoxLayout(registerGroup);

    registerUserName_ = new QLineEdit;
    registerUserName_->setPlaceholderText("Username");
    registerUserName_->setMinimumHeight(36);
    registerForm->addWidget(registerUserName_);

    registerPassword_ = new QLineEdit;
    registerPassword_->setPlaceholderText("Password");
    registerPassword_->setEchoMode(QLineEdit::Password);
    registerPassword_->setMinimumHeight(36);
    registerForm->addWidget(registerPassword_);

    registerPasswordConfirm_ = new QLineEdit;
    registerPasswordConfirm_->setPlaceholderText("Confirm password");
    registerPasswordConfirm_->setEchoMode(QLineEdit::Password);
    registerPasswordConfirm_->setMinimumHeight(36);
    registerForm->addWidget(registerPasswordConfirm_);

    registerButton_ = new QPushButton("Register");
    registerButton_->setMinimumHeight(40);
    registerButton_->setStyleSheet(
        "QPushButton { background: #FF6B35; color: white; "
        "font-weight: bold; border-radius: 4px; }"
        "QPushButton:hover { background: #FF8C5A; }");
    registerForm->addWidget(registerButton_);

    toLoginButton_ = new QPushButton("Back to Login");
    toLoginButton_->setStyleSheet(
        "QPushButton { color: #AAAAAA; border: none; }");
    registerForm->addWidget(toLoginButton_);

    registerStatus_ = new QLabel;
    registerStatus_->setAlignment(Qt::AlignCenter);
    registerStatus_->setWordWrap(true);
    registerStatus_->setStyleSheet("color: #FF4444;");
    registerForm->addWidget(registerStatus_);

    registerLayout->addWidget(registerGroup);

    stack_ = new QStackedWidget(this);
    stack_->addWidget(loginPage_);
    stack_->addWidget(registerPage_);
    stack_->setCurrentWidget(loginPage_);

    outerLayout->addWidget(stack_);

    QObject::connect(loginButton_, &QPushButton::clicked,
                     this, &LoginScene::onLoginClicked);
    QObject::connect(registerButton_, &QPushButton::clicked,
                     this, &LoginScene::onRegisterClicked);
    QObject::connect(toRegisterButton_, &QPushButton::clicked,
                     this, &LoginScene::switchToRegister);
    QObject::connect(toLoginButton_, &QPushButton::clicked,
                     this, &LoginScene::switchToLogin);

    QObject::connect(loginPassword_, &QLineEdit::returnPressed,
                     this, &LoginScene::onLoginClicked);
    QObject::connect(registerPasswordConfirm_, &QLineEdit::returnPressed,
                     this, &LoginScene::onRegisterClicked);
}

void LoginScene::setupNetworkHandlers()
{
    GameClient* client = mainWindow_->getClient();

    QObject::connect(client, &GameClient::connected, this, [this](){
        const PendingAction action = pendingAction_;
        const QString userName = pendingUserName_;
        const QString password = pendingPassword_;

        pendingAction_ = PendingAction::kNone;
        pendingUserName_.clear();
        pendingPassword_.clear();

        if(action == PendingAction::kLogin)
        {
            sendLogin(userName, password);
        }
        else if(action == PendingAction::kRegister)
        {
            sendRegister(userName, password);
        }
    });

    QObject::connect(client, &GameClient::errorOccurred, this, [this](const QString& message){
        pendingAction_ = PendingAction::kNone;

        QLabel* status = stack_->currentWidget() == loginPage_ ? loginStatus_ : registerStatus_;

        status->setStyleSheet("color: #FF4444;");
        status->setText(message);
    });

    client->registerHandler(GameProtocol::MSG_LOGIN_RSP, [this](BinaryReader& reader){
        GameMessages::LoginResponse response;
        if(!GameMessages::decode(reader, response))
        {
            loginStatus_->setText("Malformed login response");
            return;
        }

        if(!response.success_)
        {
            loginStatus_->setText(QString::fromStdString(response.errorMessage_));
            return;
        }

        mainWindow_->setUserInfo(response.userId_, QString::fromStdString(response.userName_));

        loginStatus_->setStyleSheet("color: #44FF44;");
        loginStatus_->setText(QString("Welcome, %1.").arg(QString::fromStdString(response.userName_)));
    });

    client->registerHandler(GameProtocol::MSG_REGISTER_RSP, [this](BinaryReader& reader){
        GameMessages::RegisterResponse response;
        if (!GameMessages::decode(reader, response))
        {
            registerStatus_->setText("Malformed register response");
            return;
        }

        if (!response.success_)
        {
            registerStatus_->setText(QString::fromStdString(response.errorMessage_));
            return;
        }

        registerStatus_->setStyleSheet("color: #44FF44;");
        registerStatus_->setText("Account created. Returning to login...");

        QTimer::singleShot(1200, this, &LoginScene::switchToLogin);
    });
}

void LoginScene::sendLogin(const QString &userName, const QString &password)
{
    GameMessages::LoginRequest request;
    request.userName_ = userName.toStdString();
    request.password_ = password.toStdString();

    BinaryWriter payload;
    if(!GameMessages::encode(payload, request))
    {
        loginStatus_->setText("Login request is too large");
        return;
    }

    mainWindow_->getClient()->sendMessage(GameProtocol::MSG_LOGIN_REQ, payload);

    loginStatus_->setText("Signing in...");
}

void LoginScene::sendRegister(const QString &userName, const QString &password)
{
    GameMessages::RegisterRequest request;
    request.userName_ = userName.toStdString();
    request.password_ = password.toStdString();

    BinaryWriter payload;
    if (!GameMessages::encode(payload, request))
    {
        registerStatus_->setText("Register request is too large");
        return;
    }

    mainWindow_->getClient()->sendMessage(GameProtocol::MSG_REGISTER_REQ, payload);

    registerStatus_->setText("Creating account...");
}
