/********************************************************************************
** Form generated from reading UI file 'menu.ui'
**
** Created by: Qt User Interface Compiler version 5.15.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MENU_H
#define UI_MENU_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
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
    QPushButton *mm_against;
    QPushButton *cm_against;
    QPushButton *create_room;
    QPushButton *join_room;
    QLineEdit *lineEdit;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Menu)
    {
        if (Menu->objectName().isEmpty())
            Menu->setObjectName(QString::fromUtf8("Menu"));
        Menu->resize(360, 560);
        Menu->setMinimumSize(QSize(360, 560));
        Menu->setMaximumSize(QSize(360, 560));
        centralwidget = new QWidget(Menu);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        mm_against = new QPushButton(centralwidget);
        mm_against->setObjectName(QString::fromUtf8("mm_against"));
        mm_against->setGeometry(QRect(80, 60, 180, 70));
        cm_against = new QPushButton(centralwidget);
        cm_against->setObjectName(QString::fromUtf8("cm_against"));
        cm_against->setGeometry(QRect(80, 160, 180, 70));
        create_room = new QPushButton(centralwidget);
        create_room->setObjectName(QString::fromUtf8("create_room"));
        create_room->setGeometry(QRect(80, 260, 180, 70));
        join_room = new QPushButton(centralwidget);
        join_room->setObjectName(QString::fromUtf8("join_room"));
        join_room->setGeometry(QRect(80, 350, 180, 70));
        lineEdit = new QLineEdit(centralwidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(40, 440, 281, 51));
        QFont font;
        font.setPointSize(14);
        lineEdit->setFont(font);
        Menu->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Menu);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 360, 23));
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
        mm_against->setText(QCoreApplication::translate("Menu", "\350\201\224\346\234\272\345\257\271\346\210\230", nullptr));
        cm_against->setText(QCoreApplication::translate("Menu", "\344\272\272\346\234\272\345\257\271\346\210\230", nullptr));
        create_room->setText(QCoreApplication::translate("Menu", "\345\210\233\345\273\272\346\210\277\351\227\264", nullptr));
        join_room->setText(QCoreApplication::translate("Menu", "\345\212\240\345\205\245\346\210\277\351\227\264", nullptr));
        lineEdit->setPlaceholderText(QCoreApplication::translate("Menu", "\350\276\223\345\205\245\345\212\240\345\205\245\347\232\204\346\210\277\351\227\264\345\217\267\357\274\210\344\273\205\351\231\220\346\225\260\345\255\227\357\274\211", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Menu: public Ui_Menu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MENU_H
