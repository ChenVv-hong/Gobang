#include "gamelogic.h"

GameLogic::GameLogic()
{
    this->turn = NO_PIECE;
}

/*
 * 重置棋盘 准备开始游戏
 */
void GameLogic::startGame(){
    //黑子先行
    this->turn = BLACK_PIECE;
    //清空棋盘
    for(int i = 0; i < BOARD_ROW; i++){
        for(int j = 0; j < BOARD_COL; j++){
            this->board[i][j] = NO_PIECE;
        }
    }
    //获胜清空
    this->isWin = NO_WIN;
    //记录清空
    this->chessOrder.clear();
    //ai清空
    if(mode == CM_MODE){
        ai.clear();
    }
}

void GameLogic::setMode(int m){
    this->mode = m;
}

/*
 * 检查是否获胜 获胜了 则把获胜标志位更新
 */
bool GameLogic::checkWin(){
    int cnt = 0;
    int last;
    //横向连子
    for(int i = 0; i < BOARD_ROW;i++){
        for(int j = 0; j < BOARD_COL; j++){
            if(j == 0){
                last = board[i][j];
                cnt = 1;
            }else{
                if(board[i][j] == last && board[i][j] != NO_PIECE) cnt++;
                else{
                    last = board[i][j];
                    cnt = 1;
                }
            }
            if(cnt == 5){
                if(board[i][j] == BLACK_PIECE) this->isWin = BLACK_WIN;
                else if(board[i][j] == WHITE_PIECE) this->isWin = WHITE_WIN;
                return true;
            }
        }
    }
    //纵向连子
    for(int i = 0; i < BOARD_ROW;i++){
        for(int j = 0; j < BOARD_COL; j++){
            if(j == 0){
                last = board[j][i];
                cnt = 1;
            }else{
                if(board[j][i] == last && board[j][i] != NO_PIECE) cnt++;
                else{
                    last = board[j][i];
                    cnt = 1;
                }
            }
            if(cnt == 5){
                if(board[j][i] == BLACK_PIECE) this->isWin = BLACK_WIN;
                else if(board[j][i] == WHITE_PIECE) this->isWin = WHITE_WIN;
                return true;
            }
        }
    }
    //斜45度角 连子 左上半部分
    for(int i = 4; i <= 14; i++){
        for(int j = 0; j <= i; j++){
            if(j == 0){
                last = this->board[j][i - j];
                cnt = 1;
            }
            else{
                if(board[j][i - j] == last && board[j][i - j] != NO_PIECE) cnt++;
                else{
                    last = board[j][i - j];
                    cnt = 1;
                }
            }
            if(cnt == 5){
                if(board[j][i - j] == BLACK_PIECE) this->isWin = BLACK_WIN;
                else if(board[j][i - j] == WHITE_PIECE) this->isWin = WHITE_WIN;
                return true;
            }
        }
    }
    //斜45度角 连子 右下半部分
    for(int i = 1; i <= 10; i++){
        for(int j = i, k = 14; j <= 14; j++, k--){
            if(j == i){
                last = this->board[j][k];
                cnt = 1;
            }
            else{
                if(board[j][k] == last && board[j][k] != NO_PIECE) cnt++;
                else{
                    last = board[j][k];
                    cnt = 1;
                }
            }
            if(cnt == 5){
                if(board[j][k] == BLACK_PIECE) this->isWin = BLACK_WIN;
                else if(board[j][k] == WHITE_PIECE) this->isWin = WHITE_WIN;
                return true;
            }
        }
    }
    //斜135度角 连子 右上半部分
    for(int i = 0; i <= 10; i++){
       for(int j = 0; i + j <= 14; j++){
            if(j == 0){
                last = this->board[j][i + j];
                cnt = 1;
            }
            else{
                if(board[j][i + j] == last && board[j][i + j] != NO_PIECE) cnt++;
                else{
                    last = board[j][i + j];
                    cnt = 1;
                }
            }
            if(cnt == 5){
                if(board[j][i + j] == BLACK_PIECE) this->isWin = BLACK_WIN;
                else if(board[j][i + j] == WHITE_PIECE) this->isWin = WHITE_WIN;
                return true;
            }
        }
    }
    //斜135度角 连子 左下半部分
    for(int i = 1; i <= 10; i++){
        for(int j = 0; i + j <= 14; j++){
            if(j == 0){
                last = this->board[i + j][j];
                cnt = 1;
            }
            else{
                if(board[i + j][j] == last && board[i + j][j] != NO_PIECE) cnt++;
                else{
                    last = board[i + j][j];
                    cnt = 1;
                }
            }
            if(cnt == 5){
                if(board[i + j][j] == BLACK_PIECE) this->isWin = BLACK_WIN;
                else if(board[i + j][j] == WHITE_PIECE) this->isWin = WHITE_WIN;
                return true;
            }
        }
    }

    return false;
}

/*
 * 在棋盘中落子
 */
bool GameLogic::nextStep(Chess c){
    if(this->turn == c.colour){
        //轮到你的回合
        int row,col;
        row = int((c.y - 100) / 30.0 + 0.5);
        col = int((c.x - 100) / 30.0 + 0.5);
        if(mode == CM_MODE){
            //如果是人机模式 还需要将棋局同步到 ai中
            if(c.colour == BLACK_PIECE) ai.board[row][col] = 'B';
            else ai.board[row][col] = 'W';
        }

        //将模糊的chess坐标变为精准的
        c.x = 100 + col * 30;
        c.y = 100 + row * 30;
        if(this->board[row][col] == NO_PIECE){
            //落子更新到棋局
            this->board[row][col] = this->turn;
            //更新落子记录
            this->chessOrder.push_back(c);
            //检查是否获胜 获胜了 则把获胜标志位更新
            this->checkWin();
            //更新下一个回合该轮到谁
            if(this->turn == BLACK_PIECE) this->turn = WHITE_PIECE;
            else this->turn = BLACK_PIECE;
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

/*
 * 悔棋逻辑
 */
void GameLogic::drawGame(int color){
    while(!chessOrder.empty()){
        Chess c = this->chessOrder.back();
        int row,col;
        row = int((c.y - 100) / 30.0 + 0.5);
        col = int((c.x - 100) / 30.0 + 0.5);
        this->board[row][col] = NO_PIECE;
        if(mode == CM_MODE) this->ai.board[row][col] = '\0';
        this->chessOrder.pop_back();
        if(c.colour == color) {
            this->turn = color;
            break;
        }
    }
//    if(this->mode == CM_MODE){
//        Chess c = this->chessOrder.back();
//        int row,col;
//        row = int((c.y - 100) / 30.0 + 0.5);
//        col = int((c.x - 100) / 30.0 + 0.5);
//        this->board[row][col] = NO_PIECE;
//        this->ai.board[row][col] = '\0';
//        this->chessOrder.pop_back();

//        c = this->chessOrder.back();
//        row = int((c.y - 100) / 30.0 + 0.5);
//        col = int((c.x - 100) / 30.0 + 0.5);
//        this->board[row][col] = NO_PIECE;
//        this->ai.board[row][col] = '\0';
//        this->chessOrder.pop_back();

//    }else{
//        if(this->turn == mycolor){
//            Chess c = this->chessOrder.back();
//            int row,col;
//            row = int((c.y - 100) / 30.0 + 0.5);
//            col = int((c.x - 100) / 30.0 + 0.5);
//            this->board[row][col] = NO_PIECE;
//            this->chessOrder.pop_back();

//            c = this->chessOrder.back();
//            row = int((c.y - 100) / 30.0 + 0.5);
//            col = int((c.x - 100) / 30.0 + 0.5);
//            this->board[row][col] = NO_PIECE;
//            this->chessOrder.pop_back();
//        }else{
//            Chess c = this->chessOrder.back();
//            int row,col;
//            row = int((c.y - 100) / 30.0 + 0.5);
//            col = int((c.x - 100) / 30.0 + 0.5);
//            this->board[row][col] = NO_PIECE;
//            this->chessOrder.pop_back();
//        }
//    }
}
