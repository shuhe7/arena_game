QT       += core gui network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    network/GameClinet.cc \
    scenes/LoginScene.cc

HEADERS += \
    mainwindow.h \
    network/GameClient.h \
    scenes/LoginScene.h

FORMS += \
    mainwindow.ui

