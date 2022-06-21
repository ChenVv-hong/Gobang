#ifndef NET_H
#define NET_H

#include <QTcpSocket>
#include <string>
#include <iostream>

#define CONNECT 110            //匹配请求
#define CONNECT_SUCCESS 120	   //通常为服务器返回数据 表示连接/匹配成功
#define CONNECT_FAIL 130	   //通常为服务器返回数据 表示连接/匹配失败

#define SETPIECE 140           //落子信息
                               //既可以客户端发送数据 也可以为客户端接收

#define UNDO 150               //悔棋请求/悔棋确认
#define UNDO_YES 151           //悔棋同意
#define UNDO_NO 152            //悔棋拒绝

#define GAMEOVER 160           //游戏结束
#define DISCONNECT 170         //意外断开连接
#define SURRENDER 190          //投降

#define TIE 200               //求和请求/求和确认
#define TIE_YES 201           //求和同意
#define TIE_NO 202            //求和拒绝

#define MSG 300               //消息
#define RESTART 400           //重开

#define CREATE_ROOM 500       //创建房间
#define JOIN_ROOM   501       //加入房间
#define JOIN_ROOM_FAIL   502       //加入房间失败
#define CREATE_ROOM_SUCCESS 503       //创建房间成功

class Net
{
public:
    Net(QString ip, int port);
    ~Net();
    bool connect();
    void disconnect();
    void send(QString s);
    QString read();
public:
    QTcpSocket socket;
    QString IP;
    int PORT;
};

#endif // NET_H
