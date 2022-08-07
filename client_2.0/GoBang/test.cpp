#include "test.h"
#include "ui_test.h"
#include <mainwindow.h>
#include <matchwait.h>
test::test(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::test)
{
    ui->setupUi(this);



}

test::~test()
{
    delete ui;
}


void test::closeEvent(QCloseEvent *event){
    std::cout << "asdasd\n";
    event->accept();
}


void test::pressButton(){
    //    ^[A-Za-z0-9\u4e00-\u9fa5]{3,10}$  只包含中文，英文字母和数字，长度3～10之间
    //    ^[0-9a-zA-Z_]{4,12}$  只能包含字母、数字和下划线，长度在4~12之间
    QRegExp regExpRid("^[0-9]{1,12}$");
    QRegExp regExpName("^[A-Za-z0-9\u4e00-\u9fa5]{3,8}$");
    QRegExp regExpPasswd("^[0-9a-zA-Z_]{4,12}$");
    QString name = ui->lineEdit_name->text();
    QString passwd = ui->lineEdit_passwd->text();
    QString text = ui->lineEdit_rid->text();
    if(regExpName.exactMatch(name)){
        ui->name->setText("yes");
    }
    else ui->name->setText("no");

    if(regExpPasswd.exactMatch(passwd)){
       ui->passwd->setText("yes");
    }
    else{
        ui->passwd->setText("no");
    }
    if(regExpRid.exactMatch(text)){
        ui->label_3->setText("yes");
    }
    else{
        ui->label_3->setText("no");
    }

}
