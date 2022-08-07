/********************************************************************************
** Form generated from reading UI file 'playeritem.ui'
**
** Created by: Qt User Interface Compiler version 5.15.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLAYERITEM_H
#define UI_PLAYERITEM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PlayerItem
{
public:
    QLabel *label_points;
    QLabel *label_uid;
    QLabel *label_name;
    QLabel *label_num;

    void setupUi(QWidget *PlayerItem)
    {
        if (PlayerItem->objectName().isEmpty())
            PlayerItem->setObjectName(QString::fromUtf8("PlayerItem"));
        PlayerItem->resize(439, 75);
        label_points = new QLabel(PlayerItem);
        label_points->setObjectName(QString::fromUtf8("label_points"));
        label_points->setGeometry(QRect(360, 0, 81, 71));
        QFont font;
        font.setFamily(QString::fromUtf8("Noto Mono"));
        font.setPointSize(16);
        label_points->setFont(font);
        label_points->setAlignment(Qt::AlignCenter);
        label_uid = new QLabel(PlayerItem);
        label_uid->setObjectName(QString::fromUtf8("label_uid"));
        label_uid->setGeometry(QRect(60, 1, 111, 71));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Noto Mono"));
        font1.setPointSize(14);
        font1.setBold(false);
        font1.setItalic(false);
        label_uid->setFont(font1);
        label_uid->setAlignment(Qt::AlignCenter);
        label_name = new QLabel(PlayerItem);
        label_name->setObjectName(QString::fromUtf8("label_name"));
        label_name->setGeometry(QRect(180, 1, 171, 71));
        QFont font2;
        font2.setPointSize(14);
        label_name->setFont(font2);
        label_name->setScaledContents(false);
        label_name->setAlignment(Qt::AlignCenter);
        label_num = new QLabel(PlayerItem);
        label_num->setObjectName(QString::fromUtf8("label_num"));
        label_num->setGeometry(QRect(-10, 0, 81, 71));
        label_num->setFont(font);
        label_num->setScaledContents(true);
        label_num->setAlignment(Qt::AlignCenter);

        retranslateUi(PlayerItem);

        QMetaObject::connectSlotsByName(PlayerItem);
    } // setupUi

    void retranslateUi(QWidget *PlayerItem)
    {
        PlayerItem->setWindowTitle(QCoreApplication::translate("PlayerItem", "Form", nullptr));
        label_points->setText(QCoreApplication::translate("PlayerItem", "1000", nullptr));
        label_uid->setText(QCoreApplication::translate("PlayerItem", "123456789", nullptr));
        label_name->setText(QCoreApplication::translate("PlayerItem", "111111111111", nullptr));
        label_num->setText(QCoreApplication::translate("PlayerItem", "20", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PlayerItem: public Ui_PlayerItem {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLAYERITEM_H
