#include "playeritem.h"
#include "ui_playeritem.h"

PlayerItem::PlayerItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerItem)
{
    ui->setupUi(this);
}

PlayerItem::~PlayerItem()
{
    delete ui;
}

void PlayerItem::setData(QString num,QString uid, QString name, QString points){
    this->ui->label_num->setText(num);
    this->ui->label_uid->setText(uid);
    this->ui->label_name->setText(name);
    this->ui->label_points->setText(points);
}
