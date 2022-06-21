#ifndef GAMEINTERFACE_H
#define GAMEINTERFACE_H

#include <QMainWindow>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QMessageBox>
#include <net.h>
#include <player.h>
#include <gamelogic.h>


#define WAIT_START 2  //表示游戏尚未开始
#define WAIT_TIE 3   //等待和棋响应请求
#define WAIT_UNDO 4   //等待悔棋响应
#define STARTED 5      //

namespace Ui {
class GameInterface;
}

class GameInterface : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameInterface(QWidget *parent = nullptr);
    ~GameInterface();
    void setPlayer(Player p);
    void setMode(int m);
    void startGame();

private:
    Ui::GameInterface *ui;
    GameLogic *gl;
    Net *n;
    void addMsg(QString);
    void startNetGame();
private slots:
    void pressButtonBack();
    void pressButtonDraw();
    void pressButtonSurrend();
    void enter();
    //网络连接相关槽函数
    void readReady();
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError);

protected:
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *event);
public:
    Player me;
    int mycolor;
    int status;
    int type;  // type == 1 创建房间 type == 2 加入房间
    QString room_id;
//    int mode;
};

#endif // GAMEINTERFACE_H
