//
// Created by chen on 2022/7/25.
//

#include "room.h"

#include <utility>

bool room::checkWin(){
	int cnt = 0;
	int last;
	//横向连子
	for(int i = 0; i < BOARD_ROW;i++){
		for(int j = 0; j < BOARD_COL; j++){
			if(j == 0){
				last = board[i][j];
				cnt = 1;
			}else{
				if(board[i][j] == last && board[i][j] != GoBang::NO_PIECE) cnt++;
				else{
					last = board[i][j];
					cnt = 1;
				}
			}
			if(cnt == 5){
				if(board[i][j] == GoBang::BLACK) this->isWin = BLACK_WIN;
				else if(board[i][j] == GoBang::WHITE) this->isWin = WHITE_WIN;
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
				if(board[j][i] == last && board[j][i] != GoBang::NO_PIECE) cnt++;
				else{
					last = board[j][i];
					cnt = 1;
				}
			}
			if(cnt == 5){
				if(board[j][i] == GoBang::BLACK) this->isWin = BLACK_WIN;
				else if(board[j][i] == GoBang::WHITE) this->isWin = WHITE_WIN;
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
				if(board[j][i - j] == last && board[j][i - j] != GoBang::NO_PIECE) cnt++;
				else{
					last = board[j][i - j];
					cnt = 1;
				}
			}
			if(cnt == 5){
				if(board[j][i - j] == GoBang::BLACK) this->isWin = BLACK_WIN;
				else if(board[j][i - j] == GoBang::WHITE) this->isWin = WHITE_WIN;
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
				if(board[j][k] == last && board[j][k] != GoBang::NO_PIECE) cnt++;
				else{
					last = board[j][k];
					cnt = 1;
				}
			}
			if(cnt == 5){
				if(board[j][k] == GoBang::BLACK) this->isWin = BLACK_WIN;
				else if(board[j][k] == GoBang::WHITE) this->isWin = WHITE_WIN;
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
				if(board[j][i + j] == last && board[j][i + j] != GoBang::NO_PIECE) cnt++;
				else{
					last = board[j][i + j];
					cnt = 1;
				}
			}
			if(cnt == 5){
				if(board[j][i + j] == GoBang::BLACK) this->isWin = BLACK_WIN;
				else if(board[j][i + j] == GoBang::WHITE) this->isWin = WHITE_WIN;
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
				if(board[i + j][j] == last && board[i + j][j] != GoBang::NO_PIECE) cnt++;
				else{
					last = board[i + j][j];
					cnt = 1;
				}
			}
			if(cnt == 5){
				if(board[i + j][j] == GoBang::BLACK) this->isWin = BLACK_WIN;
				else if(board[i + j][j] == GoBang::WHITE) this->isWin = WHITE_WIN;
				return true;
			}
		}
	}

	return false;
}

void room::initRoom() {
	//玩家初始
	p1_fd = -1;

}

void room::initGame() {

}

room::room(std::string rid) {
	this->roomId = std::move(rid);
}

