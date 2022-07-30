//
// Created by chen on 2022/7/24.
//

#include "myUtils.h"

void myUtils::setNonBlock(int fd){
	int flag = fcntl(fd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flag);
}

void myUtils::fdAdd(int ep, int fd, bool oneShot) {
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if(oneShot) event.events |= EPOLLONESHOT;
	epoll_ctl(ep, EPOLL_CTL_ADD, fd, &event);
	setNonBlock(fd);
}

void myUtils::fdRemove(int ep, int fd) {
	epoll_ctl(ep, EPOLL_CTL_DEL, fd, nullptr);
	close(fd);
}

void myUtils::fdMode(int ep, int fd, int ev) {
	epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(ep, EPOLL_CTL_MOD, fd, &event);
}

void myUtils::printMsg() {
	std::cout << "////////////////////////////////////////////////////////////////////\n"
	             "//                          _ooOoo_                               //\n"
	             "//                         o8888888o                              //\n"
	             "//                         88\" . \"88                              //\n"
	             "//                         (| ^_^ |)                              //\n"
	             "//                         O\\  =  /O                              //\n"
	             "//                      ____/`---'\\____                           //\n"
	             "//                    .'  \\\\|     |//  `.                         //\n"
	             "//                   /  \\\\|||  :  |||//  \\                        //\n"
	             "//                  /  _||||| -:- |||||-  \\                       //\n"
	             "//                  |   | \\\\\\  -  /// |   |                       //\n"
	             "//                  | \\_|  ''\\---/''  |   |                       //\n"
	             "//                  \\  .-\\__  `-`  ___/-. /                       //\n"
	             "//                ___`. .'  /--.--\\  `. . ___                     //\n"
	             "//              .\"\" '<  `.___\\_<|>_/___.'  >'\"\".                  //\n"
	             "//            | | :  `- \\`.;`\\ _ /`;.`/ - ` : | |                 //\n"
	             "//            \\  \\ `-.   \\_ __\\ /__ _/   .-` /  /                 //\n"
	             "//      ========`-.____`-.___\\_____/___.-`____.-'========         //\n"
	             "//                           `=---='                              //\n"
	             "//      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^        //\n"
	             "//                        五子棋游戏服务器                          //\n"
	             "//            author        ChenVv     2022/07/24                 //\n"
	             "//            佛祖保佑       永不宕机     永无BUG                    //\n"
	             "////////////////////////////////////////////////////////////////////\n";
}
