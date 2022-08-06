/********************************************************************************
** Form generated from reading UI file 'menu.ui'
**
** Created by: Qt User Interface Compiler version 5.15.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MENU_H
#define UI_MENU_H

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

class Ui_Menu
{
public:
    QWidget *centralwidget;
    QPushButton *pushButton_normal;
    QPushButton *pushButton_rank;
    QPushButton *pushButton_create_room;
    QPushButton *pushButton_join_room;
    QLineEdit *lineEdit_rid;
    QListWidget *RankList;
    QLabel *lable_user_picture;
    QLabel *label;
    QLabel *label_username;
    QLabel *lable_points;
    QLabel *lable_uid;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Menu)
    {
        if (Menu->objectName().isEmpty())
            Menu->setObjectName(QString::fromUtf8("Menu"));
        Menu->resize(934, 631);
        Menu->setMinimumSize(QSize(360, 560));
        centralwidget = new QWidget(Menu);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        pushButton_normal = new QPushButton(centralwidget);
        pushButton_normal->setObjectName(QString::fromUtf8("pushButton_normal"));
        pushButton_normal->setGeometry(QRect(610, 120, 180, 70));
        pushButton_normal->setStyleSheet(QString::fromUtf8("font: 18pt \"Noto Mono\";\n"
"border:2px groove gray;\n"
"padding:2px 4px;\n"
"background-color: rgb(220, 220, 220);\n"
"border-radius:10px;"));
        pushButton_rank = new QPushButton(centralwidget);
        pushButton_rank->setObjectName(QString::fromUtf8("pushButton_rank"));
        pushButton_rank->setGeometry(QRect(610, 220, 180, 70));
        pushButton_rank->setStyleSheet(QString::fromUtf8("font: 18pt \"Noto Mono\";\n"
"background-color: rgb(220, 220, 220);\n"
"border:2px groove gray;\n"
"border-radius:10px;\n"
"padding:2px 4px;\n"
""));
        pushButton_create_room = new QPushButton(centralwidget);
        pushButton_create_room->setObjectName(QString::fromUtf8("pushButton_create_room"));
        pushButton_create_room->setGeometry(QRect(610, 320, 180, 70));
        pushButton_create_room->setStyleSheet(QString::fromUtf8("font: 18pt \"Noto Mono\";\n"
"background-color: rgb(220, 220, 220);\n"
"border:2px groove gray;\n"
"border-radius:10px;\n"
"padding:2px 4px;\n"
""));
        pushButton_join_room = new QPushButton(centralwidget);
        pushButton_join_room->setObjectName(QString::fromUtf8("pushButton_join_room"));
        pushButton_join_room->setGeometry(QRect(610, 410, 180, 70));
        pushButton_join_room->setStyleSheet(QString::fromUtf8("font: 18pt \"Noto Mono\";\n"
"background-color: rgb(220, 220, 220);\n"
"border:2px groove gray;\n"
"border-radius:10px;\n"
"padding:2px 4px;\n"
""));
        lineEdit_rid = new QLineEdit(centralwidget);
        lineEdit_rid->setObjectName(QString::fromUtf8("lineEdit_rid"));
        lineEdit_rid->setGeometry(QRect(570, 500, 281, 51));
        QFont font;
        font.setPointSize(14);
        lineEdit_rid->setFont(font);
        RankList = new QListWidget(centralwidget);
        RankList->setObjectName(QString::fromUtf8("RankList"));
        RankList->setGeometry(QRect(30, 120, 477, 461));
        lable_user_picture = new QLabel(centralwidget);
        lable_user_picture->setObjectName(QString::fromUtf8("lable_user_picture"));
        lable_user_picture->setGeometry(QRect(10, 0, 81, 71));
        lable_user_picture->setPixmap(QPixmap(QString::fromUtf8(":/image/avatar04.png")));
        lable_user_picture->setScaledContents(true);
        lable_user_picture->setWordWrap(false);
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(200, 80, 141, 41));
        label->setStyleSheet(QString::fromUtf8("font: 18pt \"Noto Mono\";\n"
"color: rgb(255, 0, 0);"));
        label->setAlignment(Qt::AlignCenter);
        label_username = new QLabel(centralwidget);
        label_username->setObjectName(QString::fromUtf8("label_username"));
        label_username->setGeometry(QRect(340, 22, 361, 41));
        label_username->setStyleSheet(QString::fromUtf8("font: 17pt \"Noto Mono\";"));
        lable_points = new QLabel(centralwidget);
        lable_points->setObjectName(QString::fromUtf8("lable_points"));
        lable_points->setGeometry(QRect(720, 22, 201, 41));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Noto Mono"));
        font1.setPointSize(17);
        font1.setBold(false);
        font1.setItalic(false);
        lable_points->setFont(font1);
        lable_points->setStyleSheet(QString::fromUtf8("font: 17pt \"Noto Mono\";"));
        lable_uid = new QLabel(centralwidget);
        lable_uid->setObjectName(QString::fromUtf8("lable_uid"));
        lable_uid->setGeometry(QRect(102, 22, 241, 41));
        lable_uid->setStyleSheet(QString::fromUtf8("font: 17pt \"Noto Mono\";"));
        Menu->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Menu);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 934, 23));
        Menu->setMenuBar(menubar);
        statusbar = new QStatusBar(Menu);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        Menu->setStatusBar(statusbar);

        retranslateUi(Menu);

        QMetaObject::connectSlotsByName(Menu);
    } // setupUi

    void retranslateUi(QMainWindow *Menu)
    {
        Menu->setWindowTitle(QCoreApplication::translate("Menu", "MainWindow", nullptr));
        pushButton_normal->setText(QCoreApplication::translate("Menu", "\345\277\253\351\200\237\345\274\200\345\247\213", nullptr));
        pushButton_rank->setText(QCoreApplication::translate("Menu", "\346\216\222\344\275\215\345\214\271\351\205\215", nullptr));
        pushButton_create_room->setText(QCoreApplication::translate("Menu", "\345\210\233\345\273\272\346\210\277\351\227\264", nullptr));
        pushButton_join_room->setText(QCoreApplication::translate("Menu", "\345\212\240\345\205\245\346\210\277\351\227\264", nullptr));
        lineEdit_rid->setPlaceholderText(QCoreApplication::translate("Menu", "\350\276\223\345\205\245\345\212\240\345\205\245\347\232\204\346\210\277\351\227\264\345\217\267\357\274\210\344\273\205\351\231\220\346\225\260\345\255\227\357\274\211", nullptr));
        lable_user_picture->setText(QString());
        label->setText(QCoreApplication::translate("Menu", "\345\205\250\346\234\215\346\216\222\345\220\215", nullptr));
        label_username->setText(QCoreApplication::translate("Menu", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        lable_points->setText(QCoreApplication::translate("Menu", "\347\247\257\345\210\206\357\274\2322000", nullptr));
        lable_uid->setText(QCoreApplication::translate("Menu", "uid:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Menu: public Ui_Menu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MENU_H
