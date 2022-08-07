/********************************************************************************
** Form generated from reading UI file 'matchwait.ui'
**
** Created by: Qt User Interface Compiler version 5.15.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MATCHWAIT_H
#define UI_MATCHWAIT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_matchwait
{
public:
    QLabel *label;

    void setupUi(QWidget *matchwait)
    {
        if (matchwait->objectName().isEmpty())
            matchwait->setObjectName(QString::fromUtf8("matchwait"));
        matchwait->resize(607, 495);
        matchwait->setMinimumSize(QSize(607, 495));
        matchwait->setMaximumSize(QSize(607, 495));
        matchwait->setStyleSheet(QString::fromUtf8(""));
        label = new QLabel(matchwait);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(0, 0, 607, 495));
        label->setPixmap(QPixmap(QString::fromUtf8(":/image/123.png")));
        label->setScaledContents(true);

        retranslateUi(matchwait);

        QMetaObject::connectSlotsByName(matchwait);
    } // setupUi

    void retranslateUi(QWidget *matchwait)
    {
        matchwait->setWindowTitle(QCoreApplication::translate("matchwait", "Form", nullptr));
        label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class matchwait: public Ui_matchwait {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MATCHWAIT_H
