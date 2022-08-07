#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <menu.h>
#include <QDebug>
#include <test.h>
#include <myprotocol.pb.h>
#include <algorithm>

namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool connectServer();


private slots:
    void pressButton();
    void switchButton();
public slots:
    void readReady();
    void connected();
    void disconnected();

private:
    Ui::MainWindow *ui;
    enum TYPE{
        LOGIN = 0,
        REGISTER = 1
    };
    TYPE t;
    Net n;
    Menu menu;
};
#endif // MAINWINDOW_H
