//
// Created by chen on 2022/7/25.
//

#include "room.h"

#include <utility>

room::room(std::string rid) {
	this->roomId = std::move(rid);
	initRoom();
}

void room::initRoom() {
	//玩家初始
	p1_fd = -1;
	p2_fd = -1;
	p1_color = GoBang::NO_PIECE;
	p2_color = GoBang::NO_PIECE;
	playerCount = 0;
	gameState = false;
}

void room::initGame() {
	memset(board, 0, sizeof board);
	whoWin = "#";
	isWin = false;
	next_color = GoBang::BLACK;
	continueGame = 0;
	if(p1_color == GoBang::BLACK) p1_color = GoBang::WHITE;
	else if(p1_color == GoBang::WHITE) p1_color = GoBang::BLACK;
	else p1_color = GoBang::NO_PIECE;

	if(p2_color == GoBang::BLACK) p2_color = GoBang::WHITE;
	else if(p2_color == GoBang::WHITE) p2_color = GoBang::BLACK;
	else p2_color = GoBang::NO_PIECE;

	if(p1_color == p2_color){
		if(p1_color == GoBang::BLACK) p1_color = GoBang::WHITE;
		else if(p1_color == GoBang::WHITE) p1_color = GoBang::BLACK;
	}

	while(!pieceOrder.empty()) pieceOrder.pop();
}


int room::addPlayer(int p_fd, GoBang::Player p, GoBang::PieceColor c) {
	if(playerCount == 0){
		p1_fd = p_fd;
		p1 = std::move(p);
		p1_color = c;
		playerCount++;
		return playerCount;
	}
	else if(playerCount == 1){
		p2_fd = p_fd;
		p2 = std::move(p);
		p2_color = c;
		playerCount++;
		return playerCount;
	}
	else{
		//可能为重联用户加入房间
		if(p.uid() == p1.uid()){
			p1_fd = p_fd;
			return -2;
		}
		else if(p.uid() == p1.uid()){
			p2_fd = p_fd;
			return -2;
		}
	}
	return -1;
}

void room::unDo(GoBang::PieceColor c) {
	//从栈中一直弹出 直到颜色为c的记录
	while(!pieceOrder.empty()){
		GoBang::Piece p = pieceOrder.top();
		board[p.x()][p.y()] = GoBang::NO_PIECE;
		pieceOrder.pop();
		if(p.color() == c){
			break;
		}
	}
}

bool room::setPiece(const GoBang::Piece& p) {
	if(p.color() == next_color){
		board[p.x()][p.y()] = p.color();
		pieceOrder.push(p);
		isWin = checkWin();
		return true;
	}
	return false;
}

int room::playerQuitRoom(const std::string &uid) {
	if(p1.uid() == uid){
		p1_fd = -1;
		p1.clear_uid();
		p1.clear_name();
		p1.clear_points();
		playerCount--;
		return playerCount;
	}
	if(p2.uid() == uid){
		p2_fd = -1;
		p2.clear_uid();
		p2.clear_name();
		p2.clear_points();
		playerCount--;
		return playerCount;
	}
	
	return -1;
}

int room::getPlayerFd(const std::string &uid) {
	if(p1.uid() == uid) return p1_fd;
	if(p2.uid() == uid) return p2_fd;
	return -1;
}

GoBang::PieceColor room::getPlayerPieceColor(const std::string &uid) {
	if(p1.uid() == uid){
		return p1_color;
	}
	if(p2.uid() == uid){
		return p2_color;
	}
	return GoBang::NO_PIECE;
}

int room::getAnotherPlayerFd(const std::string &uid) {
	if(p1.uid() == uid) return p2_fd;
	if(p2.uid() == uid) return p1_fd;
	return -1;
}

GoBang::PieceColor room::getAnotherPlayerPieceColor(const std::string &uid) {
	if(p1.uid() == uid) return p1_color;
	if(p2.uid() == uid) return p2_color;
	return GoBang::NO_PIECE;
}

GoBang::PieceColor room::getNextColor() const {
	return next_color;
}

GoBang::Border room::getBorder() {
	GoBang::Border b;
	int i = 0;
	int64_t r = 0;
	int64_t no = 0x0000;
	int64_t black = 0x0002;
	int64_t white = 0x0003;

	for(i = 0; i < 2; i++){
		for(int j = 0; j < 16; j++){
			if(board[i][j] == GoBang::NO_PIECE){
				r |= no;
			}
			else if(board[i][j] == GoBang::BLACK){
				r |= black;
			}
			else if(board[i][j] == GoBang::WHITE){
				r |= white;
			}
			no <<= 2;
			black <<= 2;
			white <<= 2;
		}
	}
	b.set_row_2(r);
	r = 0;
	no = 0x0000;
	black = 0x0002;
	white = 0x0003;
	for(;i < 4; i++){
		for(int j = 0; j < 16; j++){
			if(board[i][j] == GoBang::NO_PIECE){
				r |= no;
			}
			else if(board[i][j] == GoBang::BLACK){
				r |= black;
			}
			else if(board[i][j] == GoBang::WHITE){
				r |= white;
			}
			no <<= 2;
			black <<= 2;
			white <<= 2;
		}
	}
	b.set_row_4(r);
	r = 0;
	no = 0x0000;
	black = 0x0002;
	white = 0x0003;
	for(;i < 6; i++){
		for(int j = 0; j < 16; j++){
			if(board[i][j] == GoBang::NO_PIECE){
				r |= no;
			}
			else if(board[i][j] == GoBang::BLACK){
				r |= black;
			}
			else if(board[i][j] == GoBang::WHITE){
				r |= white;
			}
			no <<= 2;
			black <<= 2;
			white <<= 2;
		}
	}
	b.set_row_6(r);
	r = 0;
	no = 0x0000;
	black = 0x0002;
	white = 0x0003;
	for(;i < 8; i++){
		for(int j = 0; j < 16; j++){
			if(board[i][j] == GoBang::NO_PIECE){
				r |= no;
			}
			else if(board[i][j] == GoBang::BLACK){
				r |= black;
			}
			else if(board[i][j] == GoBang::WHITE){
				r |= white;
			}
			no <<= 2;
			black <<= 2;
			white <<= 2;
		}
	}
	b.set_row_8(r);
	r = 0;
	no = 0x0000;
	black = 0x0002;
	white = 0x0003;
	for(;i < 10; i++){
		for(int j = 0; j < 16; j++){
			if(board[i][j] == GoBang::NO_PIECE){
				r |= no;
			}
			else if(board[i][j] == GoBang::BLACK){
				r |= black;
			}
			else if(board[i][j] == GoBang::WHITE){
				r |= white;
			}
			no <<= 2;
			black <<= 2;
			white <<= 2;
		}
	}
	b.set_row_10(r);
	r = 0;
	no = 0x0000;
	black = 0x0002;
	white = 0x0003;
	for(;i < 12; i++){
		for(int j = 0; j < 16; j++){
			if(board[i][j] == GoBang::NO_PIECE){
				r |= no;
			}
			else if(board[i][j] == GoBang::BLACK){
				r |= black;
			}
			else if(board[i][j] == GoBang::WHITE){
				r |= white;
			}
			no <<= 2;
			black <<= 2;
			white <<= 2;
		}
	}
	b.set_row_12(r);
	r = 0;
	no = 0x0000;
	black = 0x0002;
	white = 0x0003;
	for(;i < 14; i++){
		for(int j = 0; j < 16; j++){
			if(board[i][j] == GoBang::NO_PIECE){
				r |= no;
			}
			else if(board[i][j] == GoBang::BLACK){
				r |= black;
			}
			else if(board[i][j] == GoBang::WHITE){
				r |= white;
			}
			no <<= 2;
			black <<= 2;
			white <<= 2;
		}
	}
	b.set_row_14(r);
	r = 0;
	no = 0x0000;
	black = 0x0002;
	white = 0x0003;
	for(;i < 16; i++){
		for(int j = 0; j < 16; j++){
			if(board[i][j] == GoBang::NO_PIECE){
				r |= no;
			}
			else if(board[i][j] == GoBang::BLACK){
				r |= black;
			}
			else if(board[i][j] == GoBang::WHITE){
				r |= white;
			}
			no <<= 2;
			black <<= 2;
			white <<= 2;
		}
	}
	b.set_row_16(r);
	return b;
}

void room::changeFD(const std::string& uid, int fd) {
	if(p1.uid() == uid) p1_fd = fd;
	else if(p2.uid() == uid) p2_fd = fd;
}





bool room::checkWin(){
	int cnt = 0;
	int8_t last;
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
				if(board[i][j] == GoBang::BLACK){
					if(p1_color == GoBang::BLACK) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
				else if(board[i][j] == GoBang::WHITE){
					if(p1_color == GoBang::WHITE) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
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
				if(board[j][i] == GoBang::BLACK){
					if(p1_color == GoBang::BLACK) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
				else if(board[j][i] == GoBang::WHITE){
					if(p1_color == GoBang::WHITE) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
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
				if(board[j][i - j] == GoBang::BLACK){
					if(p1_color == GoBang::BLACK) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
				else if(board[j][i - j] == GoBang::WHITE){
					if(p1_color == GoBang::WHITE) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
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
				if(board[j][k] == GoBang::BLACK){
					if(p1_color == GoBang::BLACK) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
				else if(board[j][k] == GoBang::WHITE){
					if(p1_color == GoBang::WHITE) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
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
				if(board[j][i + j] == GoBang::BLACK){
					if(p1_color == GoBang::BLACK) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
				else if(board[j][i + j] == GoBang::WHITE){
					if(p1_color == GoBang::WHITE) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
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
				if(board[i + j][j] == GoBang::BLACK){
					if(p1_color == GoBang::BLACK) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
				else if(board[i + j][j] == GoBang::WHITE){
					if(p1_color == GoBang::WHITE) this->whoWin = p1.uid();
					else this->whoWin = p2.uid();
				}
				return true;
			}
		}
	}
	return false;
}

bool room::getIsWin() const {
	return isWin;
}

const std::string &room::getWhoWin() const {
	return whoWin;
}







