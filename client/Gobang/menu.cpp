#include "menu.h"
#include "ui_menu.h"

Menu::Menu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowModality(Qt::ApplicationModal);
    //信号和槽函数的连接
    connect(this->ui->mm_against,SIGNAL(clicked()), this, SLOT(pressButtonMM()));
    connect(this->ui->cm_against,SIGNAL(clicked()), this, SLOT(pressButtonCM()));
    connect(this->ui->create_room,SIGNAL(clicked()), this, SLOT(pressButtonCR()));
    connect(this->ui->join_room,SIGNAL(clicked()), this, SLOT(pressButtonJR()));
}

Menu::~Menu()
{
    delete ui;
}

/*
 * 通过此函数 提供给另一个窗口 接口 来传值
 */
void Menu::setPlayer(Player p){
    this->me = p;
//    qDebug() << me.getNickName() << Qt::endl;
}

/*
 * 联机对战按钮槽函数
 */
void Menu::pressButtonMM(){
    GameInterface *g = new GameInterface();
    g->setPlayer(this->me);

    g->setMode(MM_MODE);
    g->show();
    g->startGame();
}

/*
 * 人机对战按钮槽函数
 */
void Menu::pressButtonCM(){
    GameInterface *g = new GameInterface();
    g->setPlayer(this->me);

    g->setMode(CM_MODE);
    g->show();
    g->startGame();

}

/*
 * 创建房间按钮槽函数
 */
void Menu::pressButtonCR(){
    GameInterface *g = new GameInterface();
    g->setPlayer(this->me);
    g->type = 1;
    g->setMode(MM_MODE);
    g->show();
    g->startGame();
}

/*
 * 加入房间按钮槽函数
 */
void Menu::pressButtonJR(){
    QString s = this->ui->lineEdit->text();
    for(int i = 0; i < s.length(); i++){
        if(s[i] < '0' || s[i] > '9'){
            QMessageBox mess(QMessageBox::Information, tr("消息"), tr("房间号仅限数字！"));
            QPushButton *quit = (mess.addButton(tr("重来"), QMessageBox::AcceptRole));
            mess.exec();
            if(mess.clickedButton() == quit){

            }
        }
    }
    GameInterface *g = new GameInterface();
    g->setPlayer(this->me);
    g->type = 2;
    g->room_id = s;
    g->setMode(MM_MODE);
    g->show();
    g->startGame();
}
