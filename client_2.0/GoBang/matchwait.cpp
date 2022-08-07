#include "matchwait.h"
#include "ui_matchwait.h"

matchwait::matchwait(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::matchwait)
{
    ui->setupUi(this);
    //设置窗口为应用程序 打开此窗口时 阻塞其他窗口
    setWindowModality(Qt::ApplicationModal);
    this->setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);
}

matchwait::~matchwait()
{
    std::cout << "~matchwait\n";
    delete ui;
}
