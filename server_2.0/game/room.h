//
// Created by chen on 2022/7/25.
//

#ifndef SERVER_2_0_ROOM_H
#define SERVER_2_0_ROOM_H

#include "../protocol/myprotocol.pb.h"
#include <stack>

#define NO_WIN  0
#define BLACK_WIN 2
#define WHITE_WIN 3


class room {
public:
	room(std::string rid);

	/**
	 * 初始化棋局
	 */
	void initGame();

	/**
	 * 添加玩家
	 * @param p_fd 玩家 fd
	 * @param p 玩家
	 * @param c 颜色
	 * @return	返回房间中的玩家个数 -1 加入失败 -2 表示玩家为断线重连的玩家
	 */
	int addPlayer(int p_fd, GoBang::Player p, GoBang::PieceColor c);

	/**
	 * 悔棋
	 * @param c 悔棋的颜色
	 */
	void unDo(GoBang::PieceColor c);
	/**
	 * 落子 更新棋盘
	 * @param p 棋子信息
	 */
	bool setPiece(const GoBang::Piece& p);
	/**
	 * 玩家退出房间
	 * @param uid
	 * @return 返回房间中的玩家个数
	 */
	int playerQuitRoom(const std::string &uid);
	/**
	 * 获取自己的 fd
	 * @param uid 玩家id
	 * @return
	 */
	int getPlayerFd(const std::string &uid);
	/**
	 * 获取自己的棋子颜色
	 * @param uid
	 * @return
	 */
	GoBang::PieceColor getPlayerPieceColor(const std::string &uid);
	/**
	 * 获取游戏中另外一位玩家的	fd
	 * @param uid 玩家id
	 * @return
	 */
	int getAnotherPlayerFd(const std::string &uid);
	/**
	 * 获取游戏中另外一位玩家的颜色
	 * @param uid
	 * @return
	 */
	GoBang::PieceColor getAnotherPlayerPieceColor(const std::string &uid);

	GoBang::PieceColor getNextColor() const;

	GoBang::Border getBorder();

	void changeFD(const std::string& uid, int fd);

	bool getIsWin() const;

	const std::string &getWhoWin() const;

private:
	/**
	 * 初始化房间 将房间内的所有 都进行初始化
	 */
	void initRoom();
	/**
	 * 下棋指令过后 检查是否有人胜利
	 * @return 有人胜利 返回true 通过isWin 来查看谁胜利  无人胜利返回 false
	 */
	bool checkWin();
public:
	//房间id
	std::string roomId;

	//玩家1相关
	int p1_fd;      //玩家1 fd
	GoBang::Player p1;  //玩家1 信息 uid name points
	GoBang::PieceColor p1_color;    //玩家1在游戏中执什么子
	//玩家2相关
	int p2_fd;      //玩家2 fd
	GoBang::Player p2;  //玩家2 信息 uid name points
	GoBang::PieceColor p2_color;    //玩家2在游戏中执什么子
	bool gameState; //该房间是否正在游戏
	int continueGame;
private:
	const int8_t BOARD_ROW = 16;
	const int8_t BOARD_COL = 16;
	//房间中玩家个数
	int playerCount;
	//棋盘
	int8_t board[16][16];
	//是否有人赢
	bool isWin;
	//谁赢
	std::string whoWin;
	//下一个该下子的颜色
	GoBang::PieceColor next_color;
	//棋局记录
	std::stack<GoBang::Piece> pieceOrder;
};


#endif //SERVER_2_0_ROOM_H
