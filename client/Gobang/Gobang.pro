QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chess.cpp \
    gameinterface.cpp \
    gamelogic.cpp \
    main.cpp \
    mainwindow.cpp \
    menu.cpp \
    net.cpp \
    player.cpp

HEADERS += \
    AI.h \
    chess.h \
    gameinterface.h \
    gamelogic.h \
    mainwindow.h \
    menu.h \
    net.h \
    player.h

FORMS += \
    gameinterface.ui \
    mainwindow.ui \
    menu.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc
