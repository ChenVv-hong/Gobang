#ifndef GAMEINTERFACE_H
#define GAMEINTERFACE_H

#include <QMainWindow>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QMessageBox>
#include <net.h>
#include <myprotocol.pb.h>

#define BOARD_COL 15
#define BOARD_ROW 15

//下面四个是在房间中 所有的状态
#define WAIT_START 2  //表示游戏尚未开始
#define WAIT_TIE 3   //表示游戏在进行时等待和棋响应请求
#define WAIT_UNDO 4   //表示游戏在进行时等待悔棋响应
#define WAIT_CONNECT 7 //表示正在等待对方重连
#define STARTED 5      //表示游戏正在进行
//下面这个是 退出了房间所有的状态
#define NO_GAME 6   //表示已近退出了房间

namespace Ui {
class GameInterface;
}

class GameInterface : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameInterface(QWidget *parent = nullptr);
    ~GameInterface();

    /**
     * @brief 收到匹配成功 调用该接口
     * @param rid 房间号
     */
    void matchSuccess(std::string &rid);
    /**
     * @brief createRoomSuccess 创建房间成功
     * @param rid 房间号
     */
    void createRoomSuccess(std::string &rid);
    /**
     * @brief joinRoomSuccess 加入房间成功
     * @param rid 房间号
     */
    void joinRoomSuccess(std::string &cname, std::string &rid);

    /**
     * @brief setPiece 接口
     * @param nextclr 下一个棋子的颜色
     * @param b 棋盘信息
     */
    void setPiece(GoBang::PieceColor &nextclr, GoBang::Border &b);

    /**
     * @brief addMsg接口
     * @param pre 前缀
     * @param t 消息
     */
    void addMsg(QString pre, QString t);
    /**
     * @brief 悔棋请求
     */
    void undoRequest();
    /**
     * @brief 求和请求
     */
    void tieRequest();
    /**
     * @brief 悔棋回应
     * @param success
     */
    void undoResponse(bool success);
    /**
     * @brief 求和回应
     * @param success
     */
    void tieResponse(bool success);
    /**
     * @brief 收到开始游戏调用该接口
     * @param m 我的信息
     * @param mclr  我的棋子颜色
     * @param c 对手信息
     * @param cclr 对手棋子颜色
     */
    void gameStart(GoBang::Player &m, GoBang::PieceColor &mclr, GoBang::Player &c, GoBang::PieceColor &cclr);
    /**
     * @brief 游戏结束
     * @param isWin 是否获胜
     * @param msg
     */
    void gameover(bool isWin, std::string &winUid ,QString msg);
    /**
     * @brief 有人退出房间
     * @param uid 退出房间的人uid
     */
    void someoneQuitRoom(std::string &uid);
    /**
     * @brief 有人断开连接
     * @param uid 断开连接的房间的人uid
     */
    void someoneDisconnect(std::string &uid);
    /**
     * @brief someoneJoinRoom 有人加入房间
     * @param name 昵称
     * @param isReconnect 是否是重连
     */
    void someoneJoinRoom(std::string name, bool isReconnect);

    /**
     * @brief 设置网络
     * @param net
     */
    void setN(Net *net);

public:
    Ui::GameInterface *ui;

private:
    GoBang::PieceColor board[BOARD_ROW][BOARD_COL];   //棋盘状态图
    Net *n;
private:

private slots:
    void pressButtonBack();
    void pressButtonDraw();
    void pressButtonSurrend();
    void enter();


protected:
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);

public:
    GoBang::Player me;      //我的玩家信息
    GoBang::PieceColor myColor; //我的棋子颜色

    GoBang::Player competitor;  //对手的玩家信息
    GoBang::PieceColor competitorColor; //对手棋子颜色

    GoBang::PieceColor nextColor;   //下一个落子颜色
    int status; //控制逻辑
    QString room_id;    //房间号



};

#endif // GAMEINTERFACE_H
