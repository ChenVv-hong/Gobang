#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //信号和槽函数连接
    connect(this->ui->pushButton, SIGNAL(clicked()), this, SLOT(pressButton()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * 进入按钮的槽函数
 */
void MainWindow::pressButton(){
    Player p(this->ui->lineEdit->text());
    Menu *menu = new Menu();
    menu->setPlayer(p);
    menu->show();
//    this->close();
}

