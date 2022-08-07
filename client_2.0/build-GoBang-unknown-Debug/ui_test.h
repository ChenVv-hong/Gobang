/********************************************************************************
** Form generated from reading UI file 'test.ui'
**
** Created by: Qt User Interface Compiler version 5.15.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEST_H
#define UI_TEST_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_test
{
public:
    QWidget *centralwidget;
    QLabel *name;
    QLabel *passwd;
    QLabel *label_3;
    QPushButton *pushButton;
    QLineEdit *lineEdit_name;
    QLineEdit *lineEdit_passwd;
    QLineEdit *lineEdit_rid;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *test)
    {
        if (test->objectName().isEmpty())
            test->setObjectName(QString::fromUtf8("test"));
        test->resize(800, 600);
        centralwidget = new QWidget(test);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        name = new QLabel(centralwidget);
        name->setObjectName(QString::fromUtf8("name"));
        name->setGeometry(QRect(40, 300, 121, 91));
        passwd = new QLabel(centralwidget);
        passwd->setObjectName(QString::fromUtf8("passwd"));
        passwd->setGeometry(QRect(350, 320, 121, 81));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(640, 310, 131, 111));
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(300, 440, 121, 71));
        lineEdit_name = new QLineEdit(centralwidget);
        lineEdit_name->setObjectName(QString::fromUtf8("lineEdit_name"));
        lineEdit_name->setGeometry(QRect(0, 170, 231, 51));
        lineEdit_passwd = new QLineEdit(centralwidget);
        lineEdit_passwd->setObjectName(QString::fromUtf8("lineEdit_passwd"));
        lineEdit_passwd->setGeometry(QRect(260, 170, 241, 51));
        lineEdit_rid = new QLineEdit(centralwidget);
        lineEdit_rid->setObjectName(QString::fromUtf8("lineEdit_rid"));
        lineEdit_rid->setGeometry(QRect(550, 170, 241, 51));
        test->setCentralWidget(centralwidget);
        menubar = new QMenuBar(test);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 23));
        test->setMenuBar(menubar);
        statusbar = new QStatusBar(test);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        test->setStatusBar(statusbar);

        retranslateUi(test);
        QObject::connect(pushButton, SIGNAL(clicked()), test, SLOT(pressButton()));

        QMetaObject::connectSlotsByName(test);
    } // setupUi

    void retranslateUi(QMainWindow *test)
    {
        test->setWindowTitle(QCoreApplication::translate("test", "MainWindow", nullptr));
        name->setText(QCoreApplication::translate("test", "TextLabel", nullptr));
        passwd->setText(QCoreApplication::translate("test", "TextLabel", nullptr));
        label_3->setText(QCoreApplication::translate("test", "TextLabel", nullptr));
        pushButton->setText(QCoreApplication::translate("test", "PushButton", nullptr));
    } // retranslateUi

};

namespace Ui {
    class test: public Ui_test {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEST_H
