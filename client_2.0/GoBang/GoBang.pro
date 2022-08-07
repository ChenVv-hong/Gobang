QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    gameinterface.cpp \
    main.cpp \
    mainwindow.cpp \
    matchwait.cpp \
    menu.cpp \
    net.cpp \
    playeritem.cpp \
    test.cpp \
    myprotocol.pb.cc

HEADERS += \
    AI.h \
    gameinterface.h \
    mainwindow.h \
    matchwait.h \
    menu.h \
    net.h \
    playeritem.h \
    test.h \
    myprotocol.pb.h

FORMS += \
    gameinterface.ui \
    mainwindow.ui \
    matchwait.ui \
    menu.ui \
    playeritem.ui \
    test.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib -lprotobuf
RESOURCES += \
    image.qrc
