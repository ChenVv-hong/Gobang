/********************************************************************************
** Form generated from reading UI file 'gameinterface.ui'
**
** Created by: Qt User Interface Compiler version 5.15.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GAMEINTERFACE_H
#define UI_GAMEINTERFACE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GameInterface
{
public:
    QWidget *centralwidget;
    QPushButton *pushButton_back;
    QPushButton *pushButton_draw;
    QPushButton *pushButton_surrend;
    QLabel *image_me;
    QLabel *image_competitor;
    QLabel *nickname_player;
    QLabel *nickname_competitor;
    QLabel *lable_me_turn;
    QLabel *lable_competitor_turn;
    QListWidget *msg_list;
    QLineEdit *lineEdit_msg;
    QPushButton *pushButton;
    QLabel *label_msg;
    QLabel *label_rid;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *GameInterface)
    {
        if (GameInterface->objectName().isEmpty())
            GameInterface->setObjectName(QString::fromUtf8("GameInterface"));
        GameInterface->resize(820, 620);
        GameInterface->setMinimumSize(QSize(820, 620));
        GameInterface->setMaximumSize(QSize(820, 620));
        centralwidget = new QWidget(GameInterface);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        pushButton_back = new QPushButton(centralwidget);
        pushButton_back->setObjectName(QString::fromUtf8("pushButton_back"));
        pushButton_back->setGeometry(QRect(540, 490, 91, 41));
        pushButton_draw = new QPushButton(centralwidget);
        pushButton_draw->setObjectName(QString::fromUtf8("pushButton_draw"));
        pushButton_draw->setGeometry(QRect(630, 490, 91, 41));
        pushButton_surrend = new QPushButton(centralwidget);
        pushButton_surrend->setObjectName(QString::fromUtf8("pushButton_surrend"));
        pushButton_surrend->setGeometry(QRect(720, 490, 91, 41));
        image_me = new QLabel(centralwidget);
        image_me->setObjectName(QString::fromUtf8("image_me"));
        image_me->setGeometry(QRect(30, 540, 51, 51));
        image_me->setPixmap(QPixmap(QString::fromUtf8(":/image/avatar04.png")));
        image_me->setScaledContents(true);
        image_competitor = new QLabel(centralwidget);
        image_competitor->setObjectName(QString::fromUtf8("image_competitor"));
        image_competitor->setGeometry(QRect(540, 20, 51, 51));
        image_competitor->setPixmap(QPixmap(QString::fromUtf8(":/image/avatar11.png")));
        image_competitor->setScaledContents(true);
        nickname_player = new QLabel(centralwidget);
        nickname_player->setObjectName(QString::fromUtf8("nickname_player"));
        nickname_player->setGeometry(QRect(100, 530, 201, 31));
        nickname_competitor = new QLabel(centralwidget);
        nickname_competitor->setObjectName(QString::fromUtf8("nickname_competitor"));
        nickname_competitor->setGeometry(QRect(360, 10, 181, 31));
        nickname_competitor->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lable_me_turn = new QLabel(centralwidget);
        lable_me_turn->setObjectName(QString::fromUtf8("lable_me_turn"));
        lable_me_turn->setGeometry(QRect(100, 560, 201, 31));
        lable_competitor_turn = new QLabel(centralwidget);
        lable_competitor_turn->setObjectName(QString::fromUtf8("lable_competitor_turn"));
        lable_competitor_turn->setGeometry(QRect(430, 50, 111, 21));
        msg_list = new QListWidget(centralwidget);
        msg_list->setObjectName(QString::fromUtf8("msg_list"));
        msg_list->setGeometry(QRect(540, 140, 251, 281));
        QFont font;
        font.setPointSize(6);
        msg_list->setFont(font);
        lineEdit_msg = new QLineEdit(centralwidget);
        lineEdit_msg->setObjectName(QString::fromUtf8("lineEdit_msg"));
        lineEdit_msg->setGeometry(QRect(540, 430, 191, 25));
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(730, 430, 61, 25));
        label_msg = new QLabel(centralwidget);
        label_msg->setObjectName(QString::fromUtf8("label_msg"));
        label_msg->setGeometry(QRect(540, 100, 101, 41));
        label_rid = new QLabel(centralwidget);
        label_rid->setObjectName(QString::fromUtf8("label_rid"));
        label_rid->setGeometry(QRect(20, 10, 191, 41));
        QFont font1;
        font1.setPointSize(18);
        label_rid->setFont(font1);
        GameInterface->setCentralWidget(centralwidget);
        menubar = new QMenuBar(GameInterface);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 820, 23));
        GameInterface->setMenuBar(menubar);
        statusbar = new QStatusBar(GameInterface);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        GameInterface->setStatusBar(statusbar);

        retranslateUi(GameInterface);

        QMetaObject::connectSlotsByName(GameInterface);
    } // setupUi

    void retranslateUi(QMainWindow *GameInterface)
    {
        GameInterface->setWindowTitle(QCoreApplication::translate("GameInterface", "MainWindow", nullptr));
        pushButton_back->setText(QCoreApplication::translate("GameInterface", "\346\202\224\346\243\213", nullptr));
        pushButton_draw->setText(QCoreApplication::translate("GameInterface", "\346\261\202\345\222\214", nullptr));
        pushButton_surrend->setText(QCoreApplication::translate("GameInterface", "\350\256\244\350\276\223", nullptr));
        image_me->setText(QString());
        image_competitor->setText(QString());
        nickname_player->setText(QCoreApplication::translate("GameInterface", "\346\230\265\347\247\260", nullptr));
        nickname_competitor->setText(QCoreApplication::translate("GameInterface", "\347\255\211\345\276\205\347\216\251\345\256\266\350\277\233\345\205\245......", nullptr));
        lable_me_turn->setText(QCoreApplication::translate("GameInterface", "\346\255\243\345\234\250\344\270\213\346\243\213...", nullptr));
        lable_competitor_turn->setText(QCoreApplication::translate("GameInterface", "\346\255\243\345\234\250\344\270\213\346\243\213...", nullptr));
        lineEdit_msg->setPlaceholderText(QCoreApplication::translate("GameInterface", "Enter", nullptr));
        pushButton->setText(QCoreApplication::translate("GameInterface", "\345\217\221\351\200\201", nullptr));
        label_msg->setText(QCoreApplication::translate("GameInterface", "\346\266\210\346\201\257\346\241\206", nullptr));
        label_rid->setText(QCoreApplication::translate("GameInterface", "\346\210\277\351\227\264\357\274\232", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GameInterface: public Ui_GameInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GAMEINTERFACE_H
