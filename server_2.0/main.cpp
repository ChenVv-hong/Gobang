#include "main.h"

#include <utility>


int ep_fd;  //epoll
int lfd;    //监听fd
int pipefd[2];  //管道读写fd 用于将信号 同一事件源
static int roomId = 1;  //房间号
std::unordered_map<std::string, room *> onlineRooms;              //在线房间  rid->room
std::unordered_map<std::string, GoBang::Player*> onlinePlayers;  //在线玩家  uid->player、
std::unordered_map<std::string, GoBang::Player*> reconnectPlayers;   //等待重连玩家
std::unordered_map<std::string, room *> playerRoom;        //玩家所在房间    房间为nullptr表示还未进入房间
std::queue<std::string> normalMatch;    //快速开始 匹配队列
std::queue<std::string> rankMatch[5];   //排位 匹配队列 0 ～ 500 500 ～ 1000 1000 ～ 1500 1500 ～ 2000 2000 ～ ...
std::string allFd[MAX_CLIENT];      //fd -> uid 当前socket连接中对应的玩家
sort_time_list timeList;        //定时器链表
std::unordered_map<std::string, timer *> playerTimer;   //玩家身上的定时器
thread_pool *threadPool;        //线程池
connection_pool *connectionPool;    //数据库连接池

//TODO 匹配超时

int main() {
	myUtils::printMsg();
	//线程池
	threadPool = new thread_pool(20,20, 10000, false);
	threadPool->print_msg();
	//数据库连接池
	connectionPool = connection_pool::GetInstance();
	int ret;
	initNet();
	//最大事件
	epoll_event events[MAX_EVENT_NUMBER];
	std::cout << "开始等待连接！\n";
	bool running = true;
	bool timeout = false;
	while(running){
		int ready = epoll_wait(ep_fd, events, MAX_CLIENT, -1);
		if(ready == -1 && errno != EINTR) myUtils::print_sys_error("epoll_wait error");
		for(int i = 0; i < ready; i++){
			int cfd = events[i].data.fd;
			if(cfd == lfd){
				//监听 fd用来 处理连接
				sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);
				while(true){
					ret = accept(events[i].data.fd, (sockaddr *)&client_addr, &client_len);
					if(ret == -1){
						if(errno == EINTR) continue;        //被中断 再次连接
						else{
							myUtils::print_sys_error("accept error");
						}
					}else break;
				}
				//连接成功 打印客户端 连接信息
				myUtils::printClientConnection(client_addr);
				initConnection(ret);
			}
			else if(cfd == pipefd[0] && events[i].events & EPOLLIN){
				//处理信号
				char signals[1024];
				ret = recv(cfd, signals, sizeof signals, 0);
				if(ret == -1){
					//TODO
					continue;
				}
				else if(ret == 0){
					continue;
				}
				else{
					//将触发的定时信号取出 延时处理
					for(int j = 0; j < ret; j++){
						switch (signals[j]) {
							case SIGALRM:{
								timeout = true;
								break;
							}
							case SIGTERM:{
								running = false;
								break;
							}
						}
					}
				}
			}
			else{
				if(events[i].events & EPOLLRDHUP){
					//对方关闭连接
					//TODO 断开连接并且存在游戏对局 添加定时器
				}
				else if(events[i].events & EPOLLIN){
					//可读事件
					//TODO
				}
				else if(events[i].events & EPOLLOUT){
					//可写事件
					//TODO
				}
				else{
					//异常
					//TODO
				}
			}
		}
		//如果超时了 处理超时事件
		if(timeout){
			//TODO
		}
	}
	close();
	return 0;
}

void initNet(){
	//创建socket
	lfd = socket(PF_INET, SOCK_STREAM, 0);
	if(lfd == -1) myUtils::print_sys_error("socket error!");

	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//绑定
	int ret = bind(lfd, (sockaddr *)&serv_addr, sizeof(serv_addr));
	if(ret == -1) myUtils::print_sys_error("bind error");
	//监听
	ret = listen(lfd, 5);
	if(ret == -1) myUtils::print_sys_error("listen error");
	//创建epoll
	ep_fd = epoll_create(MAX_CLIENT);
	if(ep_fd == -1) myUtils::print_sys_error("epoll_create error");

	//将lfd加入epoll
	epoll_event event;
	event.data.fd = lfd;
	event.events = EPOLLIN;
	epoll_ctl(ep_fd, EPOLL_CTL_ADD, lfd, &event);

	//TODO 定时内容
	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
	if(ret == -1) myUtils::print_sys_error("socketpair error");
	myUtils::setNonBlock(pipefd[1]);
	myUtils::fdAdd(ep_fd, pipefd[0], false);
	addSig(SIGALRM);
	addSig(SIGTERM);
	alarm(TIMESLOT);
}


void initConnection(int cfd){
	//设置非阻塞
	myUtils::setNonBlock(cfd);
	//初始化为 “#” 表示 tcp已经建立连接但是 还没有登陆
	allFd[cfd] = "#";
	//加入监听红黑树中
	myUtils::fdAdd(ep_fd, cfd, true);
	//用户和服务器进行tcp连接 设置定时 如果长时间 不发送消息 即登陆请求 自动断开连接
	addTimeTask(cfd, "#",300);
}


void sigHandle(int sig){
	int save_errno = errno;
	int msg = sig;
	send(pipefd[1], (char *)msg, 1, 0);
	errno = save_errno;
}


void addSig(int sig){
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = sigHandle;
	//如果打断 系统调用 执行完 信号函数 还会继续执行 系统调用
	sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, nullptr) != -1);
}

void addTimeTask(int cfd, std::string uid,int timeslot){
	timer *t = new timer();
	t->expire = time(nullptr) + timeslot;
	t->cb_func = cbFunc;
	t->fd = cfd;
	t->uid = std::move(uid);
	//向定时器链表 添加定时器
	timeList.addTimer(t);
}

void close(){
	threadPool->close();
	connectionPool->DestroyPool();
}

void cbFunc(int fd, std::string uid){
	if(uid == "#"){
		//已连接 但长时间未登陆 或 未注册
	}
	else if(fd == -1){
		//断线超时
	}
	else{
		//长时间不进行游戏
	}
}

void quitGame(int fd, std::string uid){
	//TODO 退出服务器相关
	if(fd != -1){
		//还没有断开连接 最后断开连接
		close(fd);
	}
}