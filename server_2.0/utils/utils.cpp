//
// Created by chen on 2022/7/24.
//

#include "utils.h"

void utils::setNonBlock(int fd){
	int flag = fcntl(fd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flag);
}

void utils::fdAdd(int ep, int fd, bool oneShot) {
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if(oneShot) event.events |= EPOLLONESHOT;
	epoll_ctl(ep, EPOLL_CTL_ADD, fd, &event);
	setNonBlock(fd);
}

void utils::fdRemove(int ep, int fd) {
	epoll_ctl(ep, EPOLL_CTL_DEL, fd, nullptr);
	close(fd);
}

void utils::fdMode(int ep, int fd, int ev) {
	epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(ep, EPOLL_CTL_MOD, fd, &event);
}
