#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <player.h>
#include <chess.h>
#include <QVector>
#include <QDebug>
#include <AI.h>

#define BOARD_COL 15
#define BOARD_ROW 15

#define BLACK_PIECE 0
#define WHITE_PIECE 1
#define NO_PIECE 2


#define MM_MODE 0      //联机对战模式
#define CM_MODE 1      //人机对战模式
#define BLACK_WIN 0
#define WHITE_WIN 1
#define NO_WIN 2


class GameLogic
{
public:
    GameLogic();
    bool checkWin();
    bool nextStep(Chess c);
    void setMode(int m);
    void startGame();
    void drawGame(int mycolor);
private:
    int turn;                          //表示这是谁的回合  0 为 黑子回合   1 为 白子回合

public:
    int board[BOARD_ROW][BOARD_COL];   //棋盘状态图
    Player player[2];                  //两个玩家  规定下标为0的为黑子  下标为1的为白子

    QVector<Chess> chessOrder;         //记录棋局  同时 为绘图提供方便
    int mode;
    int isWin;
    AI ai;

private:

};

#endif // GAMELOGIC_H
