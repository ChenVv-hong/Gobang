#include "mainwindow.h"

#include <QApplication>
#include <mainwindow.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow m;
    m.show();
    qDebug() << "asd";
    return a.exec();
    qDebug() << "asd";
}
