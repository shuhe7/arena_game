#include "mainwindow.h"
#include "../../common/GameProtocol.h"
#include "./network/GameClient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    return app.exec();
}
