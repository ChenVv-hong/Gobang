//
// Created by chen on 2022/7/24.
//

#ifndef SERVER_2_0_UTILS_H
#define SERVER_2_0_UTILS_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

class utils {

	/**
	 * 设置文件描述符非阻塞
	 * @param fd 文件描述符
	 */
	static void setNonBlock(int fd);

	/**
	 * 向指定epoll_fd中添加 fd
	 * 始终会添加 EPOLLIN | EPOLLET | EPOLLRDHUP 事件 EPOLLONESHOT 可选
	 * @param ep epoll文件描述符
	 * @param fd 需要添加入的文件描述符
	 * @param oneShot 是否添加 EPOLLONESHOT 事件
	 */
	static void fdAdd(int ep, int fd, bool oneShot);

	/**
	 * 向指定epoll_fd中移除fd 并关闭 fd
	 * @param ep epoll文件描述符
	 * @param fd 需要移除的文件描述符
	 */
	static void fdRemove(int ep, int fd);

	/**
	 * 向指定epoll_fd中重新注册fd监听事件
	 * 始终监听 EPOLLET | EPOLLONESHOT | EPOLLRDHUP 事件
	 * 需要额外的事件 则通过 ev添加
	 * @param ep epoll文件描述符
	 * @param fd 需要重新注册监听事件的文件描述符
	 * @param ev 额外事件
	 */
	static void fdMode(int ep, int fd, int ev);
};


#endif //SERVER_2_0_UTILS_H
