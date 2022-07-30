//
// Created by chen on 2022/7/26.
//

#ifndef SERVER_2_0_MAIN_H
#define SERVER_2_0_MAIN_H

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "protocol/myprotocol.pb.h"
#include "sql/sqlConnectionPool.h"
#include "threadpool/thread_pool.h"
#include "timer/timer.h"
#include "utils/myUtils.h"
#include "game/room.h"


#define PORT 22725
#define ADDRESS "127.0.0.1"
#define MAX_CLIENT 65530
#define MAX_EVENT_NUMBER 1024


/**
 * 准备服务器网络连接相关
 */
void initNet();
/**
 * 初始化连接
 */
void initConnection(int cfd);

/**
 * 信号处理函数
 * @param sig
 */
void sigHandle(int sig);

/**
 * 将信号绑定相关函数 即添加想要监听的信号
 * @param sig
 */
void addSig(int sig);
/**
 * 定时任务
 * @param fd
 */
void cbFunc(int fd, std::string uid);
/**
 * 添加定时任务
 */
void addTimeTask(int fd, std::string uid, int timeslot);
/**
 * 服务器结束 回收资源
 */
void close();
/**
 * 将玩家从 服务器 上移除
 * @param fd 当 fd == -1 说明玩家已经断开tcp连接 清除该玩家在服务器中的内容; fd 正常 那么则需要先断开 tcp连接 在 清除玩家在服务器内容
 */
void quitGame(int fd, const std::string &uid);

/**
 * 判断玩家在不在房间中
 * @param uid
 * @return
 */
bool isPlayerInRoom(const std::string &uid);

/**
 * 玩家在房间中 判断玩家是否是正在游戏中
 * 配合 bool isPlayerInRoom(std::string uid);
 * @param uid
 * @return
 */
bool isPlayerInGame(const std::string &uid);



#endif //SERVER_2_0_MAIN_H
