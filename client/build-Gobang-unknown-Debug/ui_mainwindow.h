/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *pushButton_enter;
    QLabel *label_title;
    QLabel *label_subtitle;
    QPushButton *pushButton_switch;
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QLabel *label_name;
    QLineEdit *lineEdit_name;
    QLabel *label_passwd;
    QLineEdit *lineEdit_passwd;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(620, 452);
        QFont font;
        font.setPointSize(11);
        MainWindow->setFont(font);
        MainWindow->setStyleSheet(QString::fromUtf8(""));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        pushButton_enter = new QPushButton(centralwidget);
        pushButton_enter->setObjectName(QString::fromUtf8("pushButton_enter"));
        pushButton_enter->setGeometry(QRect(240, 370, 131, 51));
        label_title = new QLabel(centralwidget);
        label_title->setObjectName(QString::fromUtf8("label_title"));
        label_title->setGeometry(QRect(110, 10, 391, 81));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Noto Mono"));
        font1.setPointSize(30);
        font1.setBold(false);
        font1.setItalic(false);
        label_title->setFont(font1);
        label_title->setStyleSheet(QString::fromUtf8("font: 30pt \"Noto Mono\";"));
        label_title->setAlignment(Qt::AlignCenter);
        label_subtitle = new QLabel(centralwidget);
        label_subtitle->setObjectName(QString::fromUtf8("label_subtitle"));
        label_subtitle->setGeometry(QRect(260, 110, 81, 41));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Noto Mono"));
        font2.setPointSize(20);
        label_subtitle->setFont(font2);
        label_subtitle->setAlignment(Qt::AlignCenter);
        pushButton_switch = new QPushButton(centralwidget);
        pushButton_switch->setObjectName(QString::fromUtf8("pushButton_switch"));
        pushButton_switch->setEnabled(true);
        pushButton_switch->setGeometry(QRect(210, 320, 201, 26));
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 0));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush);
        QBrush brush1(QColor(255, 0, 0, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush1);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Active, QPalette::Window, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
        QBrush brush2(QColor(35, 38, 41, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        QBrush brush3(QColor(126, 126, 126, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush3);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
        pushButton_switch->setPalette(palette);
        pushButton_switch->setStyleSheet(QString::fromUtf8("background-color: rgba(0, 0, 0, 0);\n"
"border:none;\n"
""));
        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(141, 170, 331, 141));
        gridLayout = new QGridLayout(layoutWidget);
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label_name = new QLabel(layoutWidget);
        label_name->setObjectName(QString::fromUtf8("label_name"));
        label_name->setStyleSheet(QString::fromUtf8("font: 18pt \"Noto Mono\";"));
        label_name->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_name, 0, 0, 1, 1);

        lineEdit_name = new QLineEdit(layoutWidget);
        lineEdit_name->setObjectName(QString::fromUtf8("lineEdit_name"));

        gridLayout->addWidget(lineEdit_name, 0, 1, 1, 1);

        label_passwd = new QLabel(layoutWidget);
        label_passwd->setObjectName(QString::fromUtf8("label_passwd"));
        label_passwd->setLayoutDirection(Qt::LeftToRight);
        label_passwd->setStyleSheet(QString::fromUtf8("font: 18pt \"Noto Mono\";"));
        label_passwd->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_passwd, 1, 0, 1, 1);

        lineEdit_passwd = new QLineEdit(layoutWidget);
        lineEdit_passwd->setObjectName(QString::fromUtf8("lineEdit_passwd"));
        lineEdit_passwd->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(lineEdit_passwd, 1, 1, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 620, 27));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        pushButton_enter->setText(QCoreApplication::translate("MainWindow", "\350\277\233    \345\205\245", nullptr));
        label_title->setText(QCoreApplication::translate("MainWindow", "\346\254\242 \344\271\220 \344\272\224 \345\255\220 \346\243\213", nullptr));
        label_subtitle->setText(QCoreApplication::translate("MainWindow", "\347\231\273\351\231\206", nullptr));
        pushButton_switch->setText(QCoreApplication::translate("MainWindow", "\346\262\241\346\234\211\350\264\246\345\217\267\357\274\237\346\263\250\345\206\214\344\270\200\344\270\252", nullptr));
        label_name->setText(QCoreApplication::translate("MainWindow", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        lineEdit_name->setPlaceholderText(QString());
        label_passwd->setText(QCoreApplication::translate("MainWindow", "\345\257\206\347\240\201\357\274\232", nullptr));
        lineEdit_passwd->setPlaceholderText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
