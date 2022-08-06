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
std::unordered_map<std::string, std::pair<int, long>>    uidFd;  //玩家对应的fd 和 登陆的时间

std::deque<std::pair<std::string, int>> normalMatch;    //快速开始 匹配队列
std::deque<std::pair<std::string, int>> rankMatch[5];   //排位 匹配队列 0 ～ 500 500 ～ 1000 1000 ～ 1500 1500 ～ 2000 2000 ～ ...
pthread_mutex_t mutex;

std::string allFd[MAX_CLIENT];      //fd -> uid 当前socket连接中对应的玩家

sort_time_list timeList;        //定时器链表
std::unordered_map<std::string, timer *> playerTimer;   //玩家身上的定时器 用于处理长时间 不进行游戏 或者 断线超时
timer* fdTimer[MAX_CLIENT];      //fd上的定时器 用于处理长时间不登陆或注册

thread_pool *threadPool;        //线程池
connection_pool *connectionPool;    //数据库连接池
std::vector<GoBang::Player> RList;  //排名数据的缓存  最多一百名


//TODO 匹配超时
//TODO BUG 两边都 断线重连超时

int main() {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	myUtils::printMsg();
	//线程池
	threadPool = new thread_pool(20,20, 10000, false);
	threadPool->print_msg();

	//数据库连接池
	connectionPool = connection_pool::GetInstance();
	connectionPool->init("localhost","root", "ChenJiaHong123!!", "GoBangServer",  3306, 10);
	//互斥量 初始化
	pthread_mutex_init(&mutex, nullptr);
	//初始化 排名缓存
	initRList();

	int ret;
	initNet();
	//最大事件
	epoll_event events[MAX_EVENT_NUMBER];
	std::cout << "开始等待连接！\n";
	bool running = true;
	bool timeout = false;
	while(running){
		int ready = epoll_wait(ep_fd, events, MAX_CLIENT, -1);
//		std:: cout << ready << "  " << errno << '\n';
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
								std::cout << "SIGALRM 信号触发\n";
								break;
							}
							case SIGINT:
							case SIGTERM:{
								running = false;
								std::cout << "SIGTERM | SIGINT 信号触发\n";
								break;
							}
						}
					}
				}
			}
			else{
				if(events[i].events & EPOLLRDHUP){
					//对方关闭连接
					//断开连接并且存在游戏对局 添加定时器
					std::string uid = allFd[cfd];

					if(uid == "#"){
						//玩家进行了连接就断开了 还没登陆
						std::cout << "玩家进行了连接就断开了\n";
						timeList.delTimer(fdTimer[cfd]);
						close(cfd);
						continue;
					}
					pthread_mutex_lock(&mutex);
					allFd[cfd] = "#";
					uidFd.erase(uid);
					if(playerRoom.count(uid) == 0 || playerRoom[uid] == nullptr || playerRoom[uid]->gameState == false){
						pthread_mutex_unlock(&mutex);
						//玩家不在房间中 或者 在房间中但是不在游戏中 断开连接，删除玩家在服务器上的所有资源
						std::cout << "玩家不在房间中 或者 在房间中但是不在游戏中 断开连接，删除玩家在服务器上的所有资源\n";
						quitGame(-1, uid);
						close(cfd);
					}
					else{
						close(cfd);
						//玩家在房间中并且正在游戏中 断开连接， 等待他断线重联
						//将他移入 重连玩家
						int anotherFd = playerRoom[uid]->getAnotherPlayerFd(uid);
						std::string anotherUid = playerRoom[uid]->getAnotherPlayer(uid).uid();
						if(onlinePlayers.count(anotherUid) > 0){
							std::cout << "玩家在房间中并且正在游戏中 断开连接， 等待他断线重联\n";
							GoBang::Player *p = onlinePlayers[uid];
							onlinePlayers[uid] = nullptr;
							onlinePlayers.erase(uid);
							reconnectPlayers[uid] = p;
							//给另外一位玩家发送 对方断开连接
							GoBang::GoBangResponse goBangResponse;
							goBangResponse.set_type(GoBang::SOMEONE_DISCONNECT);
							GoBang::SomeoneDisconnectResponse *resp = goBangResponse.mutable_someonedisconnectresp();
							resp->set_uid(uid);
							resp->set_name(p->name());
							//序列化
							std::string res = goBangResponse.SerializeAsString();
							int length = res.length();
							int *ptr = &length;
							char sendBuff[256];
							memset(sendBuff, 0, sizeof (sendBuff));
							memcpy(sendBuff, ptr, sizeof (int));
							memcpy(sendBuff + 4, res.c_str(), length);
							//发送
							send(anotherFd, sendBuff, 4 + res.length(), 0);
							//添加重连超时的定时器
							addTimeTask(-1, uid, 60, 1);
							pthread_mutex_unlock(&mutex);
						}

						if(reconnectPlayers.count(anotherUid) > 0){
							pthread_mutex_unlock(&mutex);
							//另外一位玩家已经断线
							//直接将两个玩家全部移除 服务器
							timeList.delTimer(playerTimer[anotherUid]);
							quitGame(-1, anotherUid);
							quitGame(-1, uid);
						}

					}
				}
				else if(events[i].events & EPOLLIN){
					//Reactor模式 主线程 只进行监听读写时间  读写操作和逻辑内容 交给 工作线程
					//可读事件
					task t;
					t.func = workThread;
					int *fd = new int(cfd);
					t.arg = (void *)fd;
					threadPool->add_task(t);
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
			timeList.tick();
			alarm(TIMESLOT);
			timeout = false;
			printServerMsg();
		}
	}
	close();
	google::protobuf::ShutdownProtobufLibrary();
	std::cout << "结束\n";
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

	//定时内容
	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
	if(ret == -1) myUtils::print_sys_error("socketpair error");
	myUtils::setNonBlock(pipefd[1]);
	myUtils::fdAdd(ep_fd, pipefd[0], false);
	addSig(SIGALRM);
	addSig(SIGTERM);
	addSig(SIGINT);
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
	pthread_mutex_lock(&mutex);
	addTimeTask(cfd, "#", 5 * 60, 0);
	pthread_mutex_unlock(&mutex);
}

void sigHandle(int sig){
	int save_errno = errno;
	int msg = sig;
	send(pipefd[1], (char *)&msg, 1, 0);
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

void addTimeTask(int cfd, const std::string& uid, int timeslot, int option){
	timer *t = new timer();
	t->expire = time(nullptr) + timeslot;
	t->cb_func = cbFunc;
	t->fd = cfd;
	t->uid = uid;
	if(option == 0) fdTimer[cfd] = t;
	else if(option == 1) playerTimer[uid] = t;
	//向定时器链表 添加定时器
	timeList.addTimer(t);
}

void close(){
	threadPool->close();
	delete threadPool;
	connectionPool->DestroyPool();
	pthread_mutex_destroy(&mutex);
	close(lfd);
	close(pipefd[0]);
	close(pipefd[1]);
}

void cbFunc(int fd, std::string uid){

	if(uid == "#"){
		//已连接 但长时间未登陆 或 未注册
		std::cout << "已连接 但长时间未登陆 或 未注册\n";
		quitGame(fd, uid);
		fdTimer[fd] = nullptr;
	}
	else if(fd == -1){
		//断线超时
		std::cout << "断线超时\n";
		quitGame(fd, uid);

		pthread_mutex_lock(&mutex);
		playerTimer[uid] = nullptr;
		pthread_mutex_unlock(&mutex);
	}
	else{
		//长时间不进行游戏
		std::cout << "长时间不进行游戏\n";
		quitGame(fd, uid);

		pthread_mutex_lock(&mutex);
		playerTimer[uid] = nullptr;
		pthread_mutex_unlock(&mutex);
	}

}

void quitGame(int fd, std::string &uid){
	//退出服务器相关
	//将内存中的数据写到数据库
	UserDao userDao;
	std::string name;
	//删除玩家资源
	pthread_mutex_lock(&mutex);
	if(onlinePlayers.count(uid) != 0 && onlinePlayers[uid] != nullptr) {
		//玩家为 在线玩家
		name = onlinePlayers[uid]->name();
		userDao.updateUser(connectionPool, uid, onlinePlayers[uid]->points());
		delete onlinePlayers[uid];
		onlinePlayers.erase(uid);
	}

	if(reconnectPlayers.count(uid) != 0 && reconnectPlayers[uid] != nullptr){
		//玩家为 断线重连玩家
		//重连超时 直接判输 另外一位玩家胜
		int anotherFd = playerRoom[uid]->getAnotherPlayerFd(uid);
		playerRoom[uid]->gameState = false;

		if(playerRoom[uid]->gametype == 2){
			//排位模式 结算积分 更新数据
			int points = reconnectPlayers[uid]->points();
			reconnectPlayers[uid]->set_points(points - 10);
			points = onlinePlayers[allFd[anotherFd]]->points();
			onlinePlayers[allFd[anotherFd]]->set_points(points + 10);
			updateList(*reconnectPlayers[uid]);
			updateList(*onlinePlayers[uid]);
		}

		GoBang::GoBangResponse goBangResponse;
		goBangResponse.set_type(GoBang::GAMEOVER);
		GoBang::GameOver *gameOver = goBangResponse.mutable_gameover();
		gameOver->set_iswin(true);
		gameOver->set_winuid(allFd[anotherFd]);
		gameOver->set_msg("对方重连超时！");

		//序列化
		std::string res = goBangResponse.SerializeAsString();
		int length = res.length();
		int *ptr = &length;

		char sendBuff[1024];
		memset(sendBuff, 0, sizeof (sendBuff));
		memcpy(sendBuff, ptr, sizeof (int));
		memcpy(sendBuff + 4, res.c_str(), length);
		//发送
		send(anotherFd, sendBuff, 4 + res.length(), 0);

		userDao.updateUser(connectionPool, uid, reconnectPlayers[uid]->points());
		delete reconnectPlayers[uid];
		reconnectPlayers.erase(uid);

	}

	//删除房间资源 playerRoom 和 onlineRoom
	std::string rid = "#";
	if(playerRoom.count(uid) != 0 && playerRoom[uid] != nullptr){
		//玩家在房间中
		rid = playerRoom[uid]->roomId;
		playerRoom[uid] = nullptr;
		playerRoom.erase(uid);
	}
	//TODO 思考一下
	if(rid != "#" && uid != "#"){
		//玩家所在房间 那么退出房间
		int cnt = onlineRooms[rid]->playerQuitRoom(uid);
		if(cnt == 0){
			//全部退出房间了 销毁房间资源
			delete onlineRooms[rid];
			onlineRooms.erase(rid);
		}
		else{
			int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
			if(anotherFd != -1){
				//给另外一位玩家发送对方退出房间的消息
				GoBang::GoBangResponse goBangResponse;
				goBangResponse.set_type(GoBang::QUIT_ROOM);
				GoBang::QuitRoomResponse *quitRoomResponse = goBangResponse.mutable_quitroomresp();
				quitRoomResponse->set_uid(uid);
				quitRoomResponse->set_name(name);

				//序列化
				std::string res = goBangResponse.SerializeAsString();
				int length = res.length();
				int *ptr = &length;

				char sendBuff[4 + length];
				memset(sendBuff, 0, sizeof (sendBuff));
				memcpy(sendBuff, ptr, sizeof (int));
				memcpy(sendBuff + 4, res.c_str(), length);
				//发送
				send(anotherFd, sendBuff, 4 + res.length(), 0);
			}
		}
	}

	if(fd != -1){
		//还没有断开连接 最后断开连接
		allFd[fd] = "#";
		uidFd.erase(uid);
		close(fd);
	}
	pthread_mutex_unlock(&mutex);
}

void printServerMsg(){
	pthread_mutex_lock(&mutex);
	std::cout << "在线玩家：\n";
	std::cout << "uid\tname\n";
	for(auto it : onlinePlayers){
		std::cout << it.second->uid() << "\t" << it.second->name() << '\n';
	}

	std::cout << "在线房间：\n";
	std::cout << "rid\tgamestate\n";
	for(auto it : onlineRooms){
		std::cout << it.second->roomId << "\t" << it.second->gameState << '\n';
	}

	std::cout << "等待连接的玩家：\n";
	std::cout << "uid\tname\n";
	for(auto it : reconnectPlayers){
		std::cout << it.second->uid() << "\t" << it.second->name() << '\n';
	}

	std::cout << "玩家所在房间：\n";
	std::cout << "uid\trid\t gamestat\n";
	for(auto it :playerRoom){
		std::cout << it.first << "\t";
		if(it.second != nullptr){
			std::cout << it.second->roomId << "\t" << it.second->gameState << '\n';
		}
		else std::cout << '\n';

	}

	pthread_mutex_unlock(&mutex);
}

void* workThread(void *arg){
	int fd = *(int *)arg;
	delete((int *)arg);
	std::cout << fd << " 处理任务\n";
	std::vector<std::string> message;
	char buff[2048];
	int readIdx = 0;
	int checkIdx = 0;
	int needRead = 0;
	int Length = 0;
	int *ptr_r = &Length;
	//读取tcp字节流
	do{
		int ret = recv(fd, buff + readIdx, 2048 - readIdx, 0);
		if(ret == -1){
			if(errno != EAGAIN){myUtils::print_sys_error("recv error");}
		}
		else{
			readIdx += ret;
			while(checkIdx < readIdx){
				//不断读取 防止tcp沾包
				if(readIdx - checkIdx >= 4){
					memcpy(ptr_r, buff + checkIdx, sizeof (int));
					checkIdx += 4;
					if(checkIdx + Length <= readIdx){
						//数据都被读到 那么截取数据
						char m[Length + 1];
						memset(m, 0, sizeof (m));
						memcpy(m, buff + checkIdx, Length);
						message.emplace_back(std::string(m,Length));
						checkIdx += Length;
					}
					else{
						//没有读到完整数据 将已近处理的数据从缓冲区中删除 并将不完整的数据移动到开头
						int i, j;
						for(i = 0, j = checkIdx - 4; j < readIdx; i++, j++){
							//将缓冲区的数据向前移动
							buff[i] = buff[j];
						}
						//重新调整readIdx和checkIdx的位置
						readIdx = i;
						checkIdx = 0;
						break;
					}
				}
				else{
					//继续读
					break;
				}
			}

		}
	}while(checkIdx < readIdx);

	GoBang::GoBangRequest goBangRequest;
	//执行消息请求
	for(int i = 0; i < message.size(); i++){
		bool ret = goBangRequest.ParseFromString(message[i]);
		if(!ret){
			std::cout << "protobuf error--->" + message[i] << '\n';
		}
		else{

			GoBang::GoBangResponse goBangResponse;
			switch (goBangRequest.type()) {

				case(GoBang::PLAYER_MESSAGE):{
					std::cout << fd << " " <<"PLAYER_MESSAGE\n";
					//请求玩家信息
					if(goBangRequest.has_playmessagereq()){
						std::string uid = goBangRequest.playmessagereq().uid();
						goBangResponse.set_type(GoBang::PLAYER_MESSAGE);
						GoBang::PlayerMessageResponse *playerMessageResponse = goBangResponse.mutable_playmessageresp();

						GoBang::Player *p = playerMessageResponse->mutable_p();

						pthread_mutex_lock(&mutex);
						if(onlinePlayers.count(uid) > 0){
							p->set_uid(onlinePlayers[uid]->uid());
							p->set_name(onlinePlayers[uid]->name());
							p->set_points(onlinePlayers[uid]->points());
						}
						else{
							p->set_uid("#");
						}
						pthread_mutex_unlock(&mutex);

						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;
						char sendBuff[4 + r.length()];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(fd,sendBuff,4 + r.length(),0);
					}
					break;
				}

				case(GoBang::LOGIN):{
					std::cout << fd << " " <<"LOGIN\n";
					//登陆
					if(goBangRequest.has_loginreq()){
						std::string username = goBangRequest.loginreq().name();
						std::string passwd = goBangRequest.loginreq().passwd();
						UserDao userDao;
						bool success = false;
						//查询是否存在用户
						GoBang::Player p = userDao.queryOneUser(connectionPool, username, passwd, success);

						goBangResponse.set_type(GoBang::LOGIN);
						if(success){
							//存在用户
							//自定义消息类型 通过以下赋值
							GoBang::LoginResponse *loginResponse = goBangResponse.mutable_loginresp();

							pthread_mutex_lock(&mutex);

							if(onlinePlayers.count(p.uid()) == 0){
								//登陆成功
								loginResponse->set_success(true);
								loginResponse->set_msg("登陆成功！");

								if(reconnectPlayers.count(p.uid()) > 0){
									//属于断线用户
									loginResponse->set_isreconnect(true);
									loginResponse->set_rid(playerRoom[p.uid()]->roomId);
									delete reconnectPlayers[p.uid()];
									reconnectPlayers.erase(p.uid());

									//调整定时器 断线超时的定时器
									timeList.delTimer(playerTimer[p.uid()]);
									playerTimer[p.uid()] = nullptr;
								}
								else loginResponse->set_isreconnect(false);

								//设置为在线用户
								onlinePlayers[p.uid()] = new GoBang::Player(p);
								allFd[fd] = p.uid();
								uidFd[p.uid()] = std::pair<int,int>(fd, time(nullptr));
								//删除fd上的定时器
								timeList.delTimer(fdTimer[fd]);
								fdTimer[fd] = nullptr;
								//设置长时间不进行游戏的定时器
								addTimeTask(fd, p.uid(), 30 * 60, 1);
								pthread_mutex_unlock(&mutex);
							}
							else {
								pthread_mutex_unlock(&mutex);
								//登陆失败
								loginResponse->set_success(false);
								loginResponse->set_msg("登陆失败！该用户已在线");
							}

							loginResponse->set_uid(p.uid());
							loginResponse->set_points(p.points());
							
						}
						else{
							//不存在用户 登陆失败
							GoBang::LoginResponse *loginResponse = goBangResponse.mutable_loginresp();
							loginResponse->set_success(false);
							loginResponse->set_msg("登陆失败！用户名或密码错误");

							//调整定时器
							pthread_mutex_lock(&mutex);
							fdTimer[fd]->expire = time(nullptr) + 5 * 60;
							timeList.adjustTimer(fdTimer[fd]);
							pthread_mutex_unlock(&mutex);

						}

						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;
						char sendBuff[4 + r.length()];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(fd,sendBuff,4 + r.length(),0);
					}
					break;
				}

				case GoBang::REGISTER:{
					std::cout << fd << " " << "REGISTER\n";
					//注册
					if(goBangRequest.has_registerreq()){
						//获取发送数据
						std::string username = goBangRequest.registerreq().name();
						std::string passwd = goBangRequest.registerreq().passwd();
						UserDao userDao;

						//设置回传数据
						goBangResponse.set_type(GoBang::REGISTER);
						int rett = userDao.insertUser(connectionPool, username, passwd);
						if(rett == 1){
							//注册成功
							GoBang::RegisterResponse *registerResponse = goBangResponse.mutable_registerresp();
							registerResponse->set_success(true);
							registerResponse->set_msg("注册成功！");
						}
						else if(rett == 0){
							//注册失败
							GoBang::RegisterResponse *registerResponse = goBangResponse.mutable_registerresp();
							registerResponse->set_success(false);
							registerResponse->set_msg("注册失败！服务器错误！");
						}
						else if(rett == -1){
							GoBang::RegisterResponse *registerResponse = goBangResponse.mutable_registerresp();
							registerResponse->set_success(false);
							registerResponse->set_msg("注册失败！已有相同用户名！");
						}
						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;
						char sendBuff[4 + r.length()];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(fd,sendBuff,4 + r.length(),0);

						//调整定时器
						pthread_mutex_lock(&mutex);
						fdTimer[fd]->expire = time(nullptr) + 5 * 60;
						timeList.adjustTimer(fdTimer[fd]);
						pthread_mutex_unlock(&mutex);

					}
					break;
				}

				case GoBang::RANK_LIST:{
					std::cout << fd << " " << "RANK_LIST\n";
					if(goBangRequest.has_ranklistreq()){
						int32_t start = goBangRequest.ranklistreq().start();
						int32_t count = goBangRequest.ranklistreq().cnt();


						goBangResponse.set_type(GoBang::RANK_LIST);
						pthread_mutex_lock(&mutex);
						if(start <= RList.size()){
							for(int j = start - 1, k = 0; k < count && j <  RList.size(); j++, k++){
								GoBang::RankListResponse *rankListResponse = goBangResponse.mutable_ranklistresp();
								rankListResponse->set_success(true);
								rankListResponse->set_msg("");
								//添加repeated元素
								GoBang::Player *p = rankListResponse->add_list();
								p->set_uid(RList[j].uid());
								p->set_name(RList[j].name());
								p->set_points(RList[j].points());
							}
						}
						pthread_mutex_unlock(&mutex);

						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;
						char sendBuff[4 + r.length()];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(fd,sendBuff,4 + r.length(),0);
					}
					break;
				}

				case GoBang::NORMAL_MATCH:{
					std::cout << fd << " " << "NORMAL_MATCH\n";
					if(goBangRequest.has_normalmatchreq()){
						std::string uid = goBangRequest.normalmatchreq().uid();

						pthread_mutex_lock(&mutex);
						normalMatch.emplace_back(uid, uidFd[uid].second);
						pthread_mutex_unlock(&mutex);

						task t;
						t.arg = nullptr;
						t.func = normalMatchThread;
						threadPool->add_task(t);

						pthread_mutex_lock(&mutex);
						//删除长时间不进行游戏的定时器
						timeList.delTimer(playerTimer[uid]);
						playerTimer[uid] = nullptr;
						pthread_mutex_unlock(&mutex);
					}

					break;
				}

				case GoBang::RANK_MATCH:{
					std::cout << fd << " " << "RANK_MATCH\n";
					if(goBangRequest.has_rankmatchreq()){
						std::string uid = goBangRequest.rankmatchreq().uid();
						//排位 匹配队列 0 ～ 500 500 ～ 1000 1000 ～ 1500 1500 ～ 2000 2000 ～ ...
						pthread_mutex_lock(&mutex);
						if(onlinePlayers[uid]->points() <= 500){
							rankMatch[0].emplace_back(uid, uidFd[uid].second);
							pthread_mutex_unlock(&mutex);
							task t;
							int *a = new int(0);
							t.arg = (void *)a;
							t.func = rankMatchThread;
							threadPool->add_task(t);
						}
						else if(onlinePlayers[uid]->points() <= 1000){
							rankMatch[1].emplace_back(uid, uidFd[uid].second);
							pthread_mutex_unlock(&mutex);
							task t;
							int *a = new int(1);
							t.arg = (void *)a;
							t.func = rankMatchThread;
							threadPool->add_task(t);
						}
						else if(onlinePlayers[uid]->points() <= 1500){
							rankMatch[2].emplace_back(uid, uidFd[uid].second);
							pthread_mutex_unlock(&mutex);
							task t;
							int *a = new int(2);
							t.arg = (void *)a;
							t.func = rankMatchThread;
							threadPool->add_task(t);
						}
						else if(onlinePlayers[uid]->points() <= 2000){
							rankMatch[3].emplace_back(uid, uidFd[uid].second);
							pthread_mutex_unlock(&mutex);
							task t;
							int *a = new int(3);
							t.arg = (void *)a;
							t.func = rankMatchThread;
							threadPool->add_task(t);
						}
						else{
							rankMatch[4].emplace_back(uid, uidFd[uid].second);
							pthread_mutex_unlock(&mutex);
							task t;
							int *a = new int(4);
							t.arg = (void *)a;
							t.func = rankMatchThread;
							threadPool->add_task(t);
						}
						pthread_mutex_lock(&mutex);
						//删除长时间不进行游戏的定时器
						timeList.delTimer(playerTimer[uid]);
						playerTimer[uid] = nullptr;
						pthread_mutex_unlock(&mutex);
					}

					break;
				}

				case GoBang::CREATE_ROOM:{
					std::cout << fd << " " << "CREATE_ROOM\n";
					if(goBangRequest.has_createroomreq()){
						std::string uid = goBangRequest.createroomreq().uid();
						//获取房间号
						pthread_mutex_lock(&mutex);

						std::string rid = std::to_string(roomId);
						roomId++;

						//创建房间
						room *rm = new room(rid, 1);
						onlineRooms[rid] = rm;

						//将该玩家加入房间
						rm->addPlayer(fd, *onlinePlayers[uid], GoBang::BLACK);
						playerRoom[uid] = rm;

						//设置回传数据
						goBangResponse.set_type(GoBang::CREATE_ROOM);
						GoBang::CreateRoomResponse *createRoomResponse = goBangResponse.mutable_createroomresp();
						createRoomResponse->set_success(true);
						createRoomResponse->set_rid(rm->roomId);
						createRoomResponse->set_msg("创建成功！");

						pthread_mutex_unlock(&mutex);

						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();

						int *ptr = &length;
						char sendBuff[4 + r.length()];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(fd,sendBuff,4 + r.length(),0);

						pthread_mutex_lock(&mutex);
						//调整定时器
						timeList.delTimer(playerTimer[uid]);
						playerTimer[uid] = nullptr;
						pthread_mutex_unlock(&mutex);

					}
					break;
				}

				case GoBang::JOIN_ROOM:{
					std::cout << fd << " " << "JOIN_ROOM\n";
					//加入房间后 会立刻开始游戏
					//给另外一位玩家先发送 msg然后直接发送 开始游戏
					if(goBangRequest.has_joinroomreq()){
						std::string uid = goBangRequest.joinroomreq().uid();
						std::string rid = goBangRequest.joinroomreq().rid();

						pthread_mutex_lock(&mutex);
						if(onlineRooms.count(rid) == 0){
							pthread_mutex_unlock(&mutex);
							//不存在该房间 加入失败
							goBangResponse.set_type(GoBang::JOIN_ROOM);
							GoBang::JoinRoomResponse *joinRoomResponse = goBangResponse.mutable_joinroomresp();
							joinRoomResponse->set_success(false);
							joinRoomResponse->set_msg("不存在该房间");

							//序列化
							std::string r = goBangResponse.SerializeAsString();
							int length = r.length();
							int *ptr = &length;
							char sendBuff[4 + r.length()];
							memset(sendBuff, 0, sizeof (sendBuff));
							memcpy(sendBuff, ptr, sizeof (int));
							memcpy(sendBuff + 4, r.c_str(), length);
							//发送
							send(fd,sendBuff,4 + r.length(),0);

							pthread_mutex_lock(&mutex);
							//调整定时器
							playerTimer[uid]->expire = time(nullptr) + 30 * 60;
							timeList.adjustTimer(playerTimer[uid]);
							pthread_mutex_unlock(&mutex);
						}
						else{
							//存在该房间
							char sendBuff[4096];
							goBangResponse.set_type(GoBang::JOIN_ROOM);
							GoBang::JoinRoomResponse *joinRoomResponse = goBangResponse.mutable_joinroomresp();
							int flag = onlineRooms[rid]->addPlayer(fd, *onlinePlayers[uid], GoBang::WHITE);
							if(flag == -1){
								//加入失败
								pthread_mutex_unlock(&mutex);
								//人数满
								joinRoomResponse->set_success(false);
								joinRoomResponse->set_msg("人数已满");
								//序列化
								std::string r = goBangResponse.SerializeAsString();
								int length = r.length();

								int *ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(fd,sendBuff,4 + r.length(),0);

								pthread_mutex_lock(&mutex);
								//调整定时器
								playerTimer[uid]->expire = time(nullptr) + 30 * 60;
								timeList.adjustTimer(playerTimer[uid]);
								pthread_mutex_unlock(&mutex);
							}
							else if(flag == -2){
								//断线 加入成功
								//删除长时间不进行游戏的定时器
								timeList.delTimer(playerTimer[uid]);
								playerTimer[uid] = nullptr;
								int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
								std::string anothername = onlinePlayers[allFd[anotherFd]]->name();
								pthread_mutex_unlock(&mutex);

								joinRoomResponse->set_success(true);
								//加入成功msg 返回 对方昵称
								joinRoomResponse->set_msg(anothername);
								joinRoomResponse->set_rid(rid);
								//序列化
								std::string r = goBangResponse.SerializeAsString();
								int length = r.length();

								int *ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(fd,sendBuff,4 + r.length(),0);

								//通知另外一位玩家 有人重连加入房间
								goBangResponse.clear_type();
								goBangResponse.clear_joinroomresp();
								goBangResponse.set_type(GoBang::SOMEONE_JOIN_ROOM);
								GoBang::SomeoneJoinRoomResponse *someoneResp = goBangResponse.mutable_someonejoinroomresp();
								someoneResp->set_reconnect(true);

								pthread_mutex_lock(&mutex);
								someoneResp->set_name(onlinePlayers[uid]->name());
								pthread_mutex_unlock(&mutex);

								//序列化
								r = goBangResponse.SerializeAsString();
								length = r.length();
								ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(anotherFd,sendBuff,4 + r.length(),0);

								//发送开始游戏
								goBangResponse.clear_type();
								goBangResponse.clear_someonejoinroomresp();

								goBangResponse.set_type(GoBang::GAMESTART);
								GoBang::GameStart *gameStart = goBangResponse.mutable_gamestart();

								pthread_mutex_lock(&mutex);


								GoBang::PieceColor anotherC = onlineRooms[rid]->getAnotherPlayerPieceColor(uid);
								gameStart->set_mycolor(onlineRooms[rid]->getPlayerPieceColor(uid));
								gameStart->set_competitorcolor(anotherC);
								gameStart->set_competitorid(allFd[anotherFd]);
								gameStart->set_competitorname(onlinePlayers[allFd[anotherFd]]->name());
								gameStart->set_rid(rid);

								pthread_mutex_unlock(&mutex);

								//序列化
								r = goBangResponse.SerializeAsString();
								length = r.length();
								ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(fd,sendBuff,4 + r.length(),0);

								//发送棋局
								goBangResponse.clear_type();
								goBangResponse.clear_gamestart();
								goBangResponse.set_type(GoBang::SET_PIECE);
								GoBang::SetPieceResponse *setPieceResponse = goBangResponse.mutable_setpieceresp();
								setPieceResponse->set_success(true);

								pthread_mutex_lock(&mutex);
								setPieceResponse->set_next(onlineRooms[rid]->getNextColor());
								GoBang::Border *border = setPieceResponse->mutable_b();
								*border = onlineRooms[rid]->getBorder();

								pthread_mutex_unlock(&mutex);

								//序列化
								r = goBangResponse.SerializeAsString();
								length = r.length();
								ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(fd,sendBuff,4 + r.length(),0);

							}
							else if(flag == 2){
								playerRoom[uid] = onlineRooms[rid];
								//加入成功
								//删除长时间不进行游戏的定时器
								timeList.delTimer(playerTimer[uid]);
								playerTimer[uid] = nullptr;

								int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
								std::string myname = onlinePlayers[uid]->name();
								std::string anothername = onlinePlayers[allFd[anotherFd]]->name();
								pthread_mutex_unlock(&mutex);

								joinRoomResponse->set_success(true);
								joinRoomResponse->set_msg(anothername);
								joinRoomResponse->set_rid(rid);
								//序列化
								std::string r = goBangResponse.SerializeAsString();
								int length = r.length();

								int *ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(fd,sendBuff,4 + r.length(),0);


								//通知另外一位玩家 有人加入房间
								goBangResponse.clear_type();
								goBangResponse.clear_joinroomresp();
								goBangResponse.set_type(GoBang::SOMEONE_JOIN_ROOM);
								GoBang::SomeoneJoinRoomResponse *someoneResp = goBangResponse.mutable_someonejoinroomresp();
								someoneResp->set_reconnect(false);

								pthread_mutex_lock(&mutex);

								someoneResp->set_name(onlinePlayers[uid]->name());
								pthread_mutex_unlock(&mutex);

								//序列化
								r = goBangResponse.SerializeAsString();
								length = r.length();
								ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(anotherFd,sendBuff,4 + r.length(),0);


								pthread_mutex_lock(&mutex);
								onlineRooms[rid]->initGame();
								//发送开始游戏
								goBangResponse.clear_type();
								goBangResponse.clear_someonejoinroomresp();

								goBangResponse.set_type(GoBang::GAMESTART);
								GoBang::GameStart *gameStart = goBangResponse.mutable_gamestart();

								anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
								GoBang::PieceColor anotherC = onlineRooms[rid]->getAnotherPlayerPieceColor(uid);
								gameStart->set_mycolor(onlineRooms[rid]->getPlayerPieceColor(uid));
								gameStart->set_competitorcolor(anotherC);
								gameStart->set_competitorid(allFd[anotherFd]);
								gameStart->set_competitorname(onlinePlayers[allFd[anotherFd]]->name());
								gameStart->set_rid(rid);

								pthread_mutex_unlock(&mutex);

								//序列化
								r = goBangResponse.SerializeAsString();
								length = r.length();
								ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(fd,sendBuff,4 + r.length(),0);

								pthread_mutex_lock(&mutex);
								//发送给另外一个玩家
								gameStart->set_mycolor(anotherC);
								gameStart->set_competitorcolor(onlineRooms[rid]->getPlayerPieceColor(uid));
								gameStart->set_competitorid(uid);
								gameStart->set_competitorname(onlinePlayers[allFd[fd]]->name());
								gameStart->set_rid(rid);
								pthread_mutex_unlock(&mutex);

								//序列化
								r = goBangResponse.SerializeAsString();
								length = r.length();
								ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(anotherFd,sendBuff,4 + r.length(),0);


								//发送setpieceresponse
								goBangResponse.clear_type();
								goBangResponse.clear_gamestart();
								goBangResponse.set_type(GoBang::SET_PIECE);
								GoBang::SetPieceResponse *setPieceResponse = goBangResponse.mutable_setpieceresp();
								setPieceResponse->set_success(true);

								pthread_mutex_lock(&mutex);

								setPieceResponse->set_next(onlineRooms[rid]->getNextColor());
								GoBang::Border *border = setPieceResponse->mutable_b();
								*border = onlineRooms[rid]->getBorder();

								pthread_mutex_unlock(&mutex);

								//序列化
								r = goBangResponse.SerializeAsString();
								length = r.length();
								ptr = &length;
								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, r.c_str(), length);
								//发送
								send(fd,sendBuff,4 + r.length(),0);
								send(anotherFd,sendBuff,4 + r.length(),0);


								pthread_mutex_lock(&mutex);
								onlineRooms[rid]->gameState = true;
								pthread_mutex_unlock(&mutex);

							}
						}
					}
					break;
				}

				case GoBang::SET_PIECE:{
					std::cout << fd << " " << "SET_PIECE\n";
					char sendBuff[1024];
					if(goBangRequest.has_setpiecereq()){
						GoBang::Piece p = goBangRequest.setpiecereq().p();
						std::string rid = goBangRequest.setpiecereq().rid();

						pthread_mutex_lock(&mutex);

						onlineRooms[rid]->setPiece(p);
						//发送setpieceresponse
						goBangResponse.set_type(GoBang::SET_PIECE);
						GoBang::SetPieceResponse *setPieceResponse = goBangResponse.mutable_setpieceresp();
						setPieceResponse->set_success(true);
						setPieceResponse->set_next(onlineRooms[rid]->getNextColor());
						GoBang::Border *border = setPieceResponse->mutable_b();
						*border = onlineRooms[rid]->getBorder();
						int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(allFd[fd]);

						pthread_mutex_unlock(&mutex);
						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;

						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(fd,sendBuff,4 + r.length(),0);
						send(anotherFd,sendBuff,4 + r.length(),0);

						//检查该玩家落子后是否胜利
						pthread_mutex_lock(&mutex);
						if(onlineRooms[rid]->getIsWin()){
							onlineRooms[rid]->gameState = false;
							//winUid即本身的
							std::string winUid = onlineRooms[rid]->getWhoWin();
							if(onlineRooms[rid]->gametype == 2){
								//排位模式 更新数据
								int points = onlinePlayers[winUid]->points();
								onlinePlayers[winUid]->set_points(points + 10);
								points = onlinePlayers[allFd[anotherFd]]->points();
								onlinePlayers[allFd[anotherFd]]->set_points(points - 10);
								updateList(*onlinePlayers[winUid]);
								updateList(*onlinePlayers[allFd[anotherFd]]);
							}

							pthread_mutex_unlock(&mutex);
							//有人胜利 发送gameover
							goBangResponse.clear_type();
							goBangResponse.clear_setpieceresp();
							goBangResponse.set_type(GoBang::GAMEOVER);
							GoBang::GameOver *gameOver = goBangResponse.mutable_gameover();
							gameOver->set_iswin(true);
							gameOver->set_winuid(winUid);
							//序列化
							r = goBangResponse.SerializeAsString();
							length = r.length();
							ptr = &length;

							memset(sendBuff, 0, sizeof (sendBuff));
							memcpy(sendBuff, ptr, sizeof (int));
							memcpy(sendBuff + 4, r.c_str(), length);
							//发送
							send(fd, sendBuff, 4 + r.length(), 0);

							//发送给另外一位玩家
							gameOver->set_iswin(false);
							gameOver->set_winuid(winUid);
							//序列化
							r = goBangResponse.SerializeAsString();
							length = r.length();
							ptr = &length;

							memset(sendBuff, 0, sizeof (sendBuff));
							memcpy(sendBuff, ptr, sizeof (int));
							memcpy(sendBuff + 4, r.c_str(), length);
							//发送
							send(anotherFd, sendBuff, 4 + r.length(), 0);

						}
						else{
							pthread_mutex_unlock(&mutex);
						}

					}
					break;
				}

				case GoBang::UNDO:{
					std::cout << fd << " " << "UNDO\n";
					if(goBangRequest.has_undoreq()){
						//悔棋请求 直接转发
						std::string uid = goBangRequest.undoreq().uid();
						std::string rid = goBangRequest.undoreq().rid();

						goBangResponse.set_type(GoBang::UNDO);
						GoBang::UndoRequest *undoRequest = goBangResponse.mutable_undoreq();
						undoRequest->set_uid(uid);
						undoRequest->set_rid(rid);


						pthread_mutex_lock(&mutex);
						int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
						pthread_mutex_unlock(&mutex);

						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;
						char sendBuff[512];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(anotherFd, sendBuff, 4 + length, 0);
					}
					if(goBangRequest.has_undoresp()){
						//悔棋响应直接转发
						bool success = goBangRequest.undoresp().success();
						std::string uid = goBangRequest.undoresp().uid();
						std::string rid = goBangRequest.undoresp().rid();

						goBangResponse.set_type(GoBang::UNDO);
						GoBang::UndoResponse *undoResp = goBangResponse.mutable_undoresp();
						undoResp->set_success(success);
						undoResp->set_uid(uid);
						undoResp->set_rid(rid);

						std::cout << "undoresp--" << undoResp->success() << '\n';

						pthread_mutex_lock(&mutex);
						int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
						pthread_mutex_unlock(&mutex);

						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;
						char sendBuff[512];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(anotherFd, sendBuff, 4 + length, 0);
						std::cout << "undoresp--4--" << length << '\n';
						if(success){

							pthread_mutex_lock(&mutex);
							//同意悔棋
							GoBang::PieceColor anotherC = onlineRooms[rid]->getAnotherPlayerPieceColor(uid);
							onlineRooms[rid]->unDo(anotherC);

							GoBang::Border b = onlineRooms[rid]->getBorder();
							GoBang::PieceColor nextColor = onlineRooms[rid]->getNextColor();

							//发送setpieceresponse
							goBangResponse.clear_type();
							goBangResponse.clear_undoresp();
							goBangResponse.set_type(GoBang::SET_PIECE);
							GoBang::SetPieceResponse *setPieceResponse = goBangResponse.mutable_setpieceresp();
							setPieceResponse->set_success(true);
							setPieceResponse->set_next(nextColor);
							GoBang::Border *border = setPieceResponse->mutable_b();
							*border = b;

							pthread_mutex_unlock(&mutex);

							//序列化
							std::string r_2 = goBangResponse.SerializeAsString();
							int length_2 = r_2.length();
							int *ptr_2 = &length_2;

							char sendBuff_2[4 + length_2];
							memset(sendBuff_2, 0, sizeof (sendBuff_2));
							memcpy(sendBuff_2, ptr_2, sizeof (int));
							memcpy(sendBuff_2 + 4, r_2.c_str(), length_2);
							//发送
							send(fd,sendBuff_2,4 + r_2.length(),0);
							send(anotherFd,sendBuff_2,4 + length_2,0);
						}
						
					}
					break;
				}

				case GoBang::TIE:{
					std::cout << fd << " " << "TIE\n";
					if(goBangRequest.has_tiereq()){
						//求和请求 转发给另一个用户
						std::string uid = goBangRequest.tiereq().uid();
						std::string rid = goBangRequest.tiereq().rid();

						goBangResponse.set_type(GoBang::TIE);
						GoBang::TieRequest *tieRequest = goBangResponse.mutable_tieres();
						tieRequest->set_uid(uid);
						tieRequest->set_rid(rid);

						pthread_mutex_lock(&mutex);
						int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
						pthread_mutex_unlock(&mutex);

						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;
						char sendBuff[512];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(anotherFd, sendBuff, 4 + length, 0);
					}
					if(goBangRequest.has_tieresp()) {
						//求和响应 直接转发
						bool success = goBangRequest.tieresp().success();
						std::string uid = goBangRequest.tieresp().uid();
						std::string rid = goBangRequest.tieresp().rid();

						goBangResponse.set_type(GoBang::TIE);
						GoBang::TieResponse *tieResponse = goBangResponse.mutable_tieresp();
						tieResponse->set_success(success);
						tieResponse->set_uid(uid);
						tieResponse->set_rid(rid);

						pthread_mutex_lock(&mutex);
						int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
						pthread_mutex_unlock(&mutex);

						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;
						char sendBuff[512];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(anotherFd, sendBuff, 4 + length, 0);

						if (success) {
							//同意求和 游戏结束
							pthread_mutex_lock(&mutex);
							onlineRooms[rid]->gameState = false;
							pthread_mutex_unlock(&mutex);
							//发送gameover
							goBangResponse.set_type(GoBang::GAMEOVER);
							GoBang::GameOver *gameOver = goBangResponse.mutable_gameover();
							gameOver->set_iswin(true);
							gameOver->set_winuid("#");

							//序列化
							std::string r = goBangResponse.SerializeAsString();
							int length = r.length();
							int *ptr = &length;
							char sendBuff[4 + length];
							memset(sendBuff, 0, sizeof (sendBuff));
							memcpy(sendBuff, ptr, sizeof (int));
							memcpy(sendBuff + 4, r.c_str(), length);
							//发送
							send(fd, sendBuff, 4 + r.length(), 0);
							send(anotherFd, sendBuff, 4 + r.length(), 0);

						}
					}
					break;
				}

				case GoBang::SURRENDER:{
					std::cout << fd << " " << "SURRENDER\n";
					char sendBuff[2048];
					//一方投降 发送gameover 并对人物积分进行相应的增减
					if(goBangRequest.has_surrenderreq()){
						std::string uid = goBangRequest.surrenderreq().uid();
						std::string rid = goBangRequest.surrenderreq().rid();

						pthread_mutex_lock(&mutex);
						int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
						onlineRooms[rid]->gameState = false;
						if(onlineRooms[rid]->gametype == 2){
							//更新数据
							int points = onlinePlayers[uid]->points();
							onlinePlayers[uid]->set_points(points - 10);
							points = onlinePlayers[allFd[anotherFd]]->points();
							onlinePlayers[allFd[anotherFd]]->set_points(points + 10);

							updateList(*onlinePlayers[uid]);
							updateList(*onlinePlayers[allFd[anotherFd]]);
						}

						pthread_mutex_unlock(&mutex);

						//发送
						goBangResponse.set_type(GoBang::GAMEOVER);
						GoBang::GameOver *gameOver = goBangResponse.mutable_gameover();
						gameOver->set_iswin(true);
						gameOver->set_winuid(allFd[anotherFd]);

						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;


						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(anotherFd, sendBuff, 4 + r.length(), 0);


						gameOver->set_iswin(false);
						//序列化
						r = goBangResponse.SerializeAsString();
						length = r.length();
						ptr = &length;
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						//发送
						send(fd, sendBuff, 4 + r.length(), 0);

					}
					break;
				}

				case GoBang::MSG:{
					std::cout << fd << " " << "MSG\n";
					//直接转发
					if(goBangRequest.has_messg()){
						std::string uid = goBangRequest.messg().uid();
						std::string rid = goBangRequest.messg().rid();
						std::string name = goBangRequest.messg().name();
						std::string msg = goBangRequest.messg().msg();

						pthread_mutex_lock(&mutex);
						int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
						pthread_mutex_unlock(&mutex);

						goBangResponse.set_type(GoBang::MSG);
						GoBang::Messg *messg = goBangResponse.mutable_messg();
						messg->set_uid(uid);
						messg->set_name(name);
						messg->set_rid(rid);
						messg->set_msg(msg);
						std::cout << "msg---" << name << ":" << msg << '\n';
						std::cout << "msg---?" << message[i].length() << '\n';
						//序列化
						std::string r = goBangResponse.SerializeAsString();
						int length = r.length();
						int *ptr = &length;
						char sendBuff[2048];
						memset(sendBuff, 0, sizeof (sendBuff));
						memcpy(sendBuff, ptr, sizeof (int));
						memcpy(sendBuff + 4, r.c_str(), length);
						send(anotherFd, sendBuff, 4 + length, 0);
					}
					break;
				}

				case GoBang::CONTINUE_GAME:{
					std::cout << fd << " " << "CONTINUE_GAME\n";
					//将room中 continue + 1 如果 continue == 2 那么发送startgame开始游戏
					char sendBuff[4096];
					if(goBangRequest.has_continuegamereq()){
						std::string uid = goBangRequest.continuegamereq().uid();
						std::string rid = goBangRequest.continuegamereq().rid();

						pthread_mutex_lock(&mutex);
						onlineRooms[rid]->continueGame++;
						if(onlineRooms[rid]->continueGame == 2 && onlineRooms[rid]->getPlayerCount() == 2){
							//两个人都想继续游戏
							onlineRooms[rid]->initGame();

							//发送开始游戏startgame

							goBangResponse.set_type(GoBang::GAMESTART);
							GoBang::GameStart *gameStart = goBangResponse.mutable_gamestart();

							int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
							GoBang::PieceColor anotherC = onlineRooms[rid]->getAnotherPlayerPieceColor(uid);
							GoBang::PieceColor myC = onlineRooms[rid]->getPlayerPieceColor(uid);
							std::string myName = onlinePlayers[allFd[fd]]->name();
							std::string anotherName = onlinePlayers[allFd[anotherFd]]->name();

							gameStart->set_mycolor(myC);
							gameStart->set_competitorcolor(anotherC);
							gameStart->set_competitorid(allFd[anotherFd]);
							gameStart->set_competitorname(anotherName);
							gameStart->set_rid(rid);

							pthread_mutex_unlock(&mutex);
							//序列化
							std::string r = goBangResponse.SerializeAsString();
							int length = r.length();
							int *ptr = &length;

							memset(sendBuff, 0, sizeof (sendBuff));
							memcpy(sendBuff, ptr, sizeof (int));
							memcpy(sendBuff + 4, r.c_str(), length);
							//发送
							send(fd,sendBuff,4 + r.length(),0);

							//发送给另外一个玩家
							gameStart->set_mycolor(anotherC);
							gameStart->set_competitorcolor(myC);
							gameStart->set_competitorid(uid);
							gameStart->set_competitorname(myName);
							gameStart->set_rid(rid);

							//序列化
							r = goBangResponse.SerializeAsString();
							length = r.length();
							ptr = &length;
							memset(sendBuff, 0, sizeof (sendBuff));
							memcpy(sendBuff, ptr, sizeof (int));
							memcpy(sendBuff + 4, r.c_str(), length);
							//发送
							send(anotherFd,sendBuff,4 + r.length(),0);

							pthread_mutex_lock(&mutex);
							//发送setpieceresponse
							GoBang::Border b = onlineRooms[rid]->getBorder();
							GoBang::PieceColor nextColor = onlineRooms[rid]->getNextColor();

							goBangResponse.clear_type();
							goBangResponse.clear_gamestart();
							goBangResponse.set_type(GoBang::SET_PIECE);
							GoBang::SetPieceResponse *setPieceResponse = goBangResponse.mutable_setpieceresp();
							setPieceResponse->set_success(true);
							setPieceResponse->set_next(nextColor);
							GoBang::Border *border = setPieceResponse->mutable_b();
							*border = b;
							pthread_mutex_unlock(&mutex);
							//序列化
							r = goBangResponse.SerializeAsString();
							length = r.length();
							ptr = &length;
							memset(sendBuff, 0, sizeof (sendBuff));
							memcpy(sendBuff, ptr, sizeof (int));
							memcpy(sendBuff + 4, r.c_str(), length);
							//发送
							send(fd,sendBuff,4 + r.length(),0);
							send(anotherFd,sendBuff,4 + r.length(),0);

						}
						else pthread_mutex_unlock(&mutex);
					}
					break;
				}

				case GoBang::QUIT_ROOM:{
					std::cout << fd << " " << "QUIT_ROOM\n";
					//退出房间后 一个房间中的人数 = 0 消除该房间
					char sendBuff[2048];
					if(goBangRequest.has_quitroomreq()){
						std::string uid = goBangRequest.quitroomreq().uid();
						std::string rid = goBangRequest.quitroomreq().rid();

						pthread_mutex_lock(&mutex);
						room *r = onlineRooms[rid];
						int anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
						std::string anotherUid = onlineRooms[rid]->getAnotherPlayer(uid).uid();
						int cnt = r->playerQuitRoom(uid);
						bool state = r->gameState;

						if(state){
							if(onlinePlayers.count(anotherUid) == 0){
								//说明是对方是断连状态 这时退出房间
								//直接取消该对局 将锻炼用户从服务器上删除
								timeList.delTimer(playerTimer[anotherUid]);
								pthread_mutex_unlock(&mutex);
								quitGame(-1, anotherUid);
							}
							else{
								r->gameState = false;
								pthread_mutex_unlock(&mutex);
								//正在游戏中 退出房间 直接判输 另外一位玩家赢
								//发送
								goBangResponse.set_type(GoBang::GAMEOVER);
								GoBang::GameOver *gameOver = goBangResponse.mutable_gameover();
								gameOver->set_iswin(true);
								gameOver->set_winuid(allFd[anotherFd]);
								gameOver->set_msg("对方退出房间！");

								//序列化
								std::string res = goBangResponse.SerializeAsString();
								int length = res.length();
								int *ptr = &length;


								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, res.c_str(), length);
								//发送
								send(anotherFd, sendBuff, 4 + res.length(), 0);

								pthread_mutex_lock(&mutex);
								//更新数据
								int points = onlinePlayers[uid]->points();
								onlinePlayers[uid]->set_points(points - 10);
								points = onlinePlayers[allFd[anotherFd]]->points();
								onlinePlayers[allFd[anotherFd]]->set_points(points + 10);
								updateList(*onlinePlayers[uid]);
								updateList(*onlinePlayers[allFd[anotherFd]]);
								pthread_mutex_unlock(&mutex);
							}
						}
						else{
							//不在游戏中
							if(cnt == 0){
								//全部退出房间了 销毁房间资源
								delete onlineRooms[rid];
								onlineRooms.erase(rid);
							}
							else{
								//给另外一位玩家发送对方退出房间的消息
								goBangResponse.set_type(GoBang::QUIT_ROOM);
								GoBang::QuitRoomResponse *quitRoomResponse = goBangResponse.mutable_quitroomresp();
								quitRoomResponse->set_uid(uid);
								quitRoomResponse->set_name(onlinePlayers[uid]->name());

								//序列化
								std::string res = goBangResponse.SerializeAsString();
								int length = res.length();
								int *ptr = &length;


								memset(sendBuff, 0, sizeof (sendBuff));
								memcpy(sendBuff, ptr, sizeof (int));
								memcpy(sendBuff + 4, res.c_str(), length);
								//发送
								send(anotherFd, sendBuff, 4 + res.length(), 0);
							}
							pthread_mutex_unlock(&mutex);
						}

						//添加不长时间进行游戏的定时器
						pthread_mutex_lock(&mutex);
						playerRoom[uid] = nullptr;
						playerRoom.erase(uid);
						addTimeTask(fd, uid, 30 * 60, 1);
						pthread_mutex_unlock(&mutex);
					}
					break;
				}

				case GoBang::GAMESTART:{
					//不会收到
					break;
				}

				case GoBang::GAMEOVER:{
					//不会收到
					break;
				}

				case GoBang::TYPE_INT_MIN_SENTINEL_DO_NOT_USE_:{
					break;
				}

				case GoBang::TYPE_INT_MAX_SENTINEL_DO_NOT_USE_:{
					break;
				}
			}
		}
	}
	//重新注册读事件
	myUtils::fdMode(ep_fd, fd, EPOLLIN);
	return nullptr;
}

void* normalMatchThread(void *arg){
	//uid time
	std::pair<std::string, int> p[2];
	int index = 0;
	int length = 0;
	int *ptr = &length;
	char sendBuff[4096];
	pthread_mutex_lock(&mutex);
	while(true){
//		pthread_mutex_lock(&mutex);
		if(normalMatch.empty()){
			//凑不出两个人 将已近取出的一个人放回去
			if(index == 1) normalMatch.push_front(p[index - 1]);
			break;
		}
		if(normalMatch.size() == 1 && index == 0) {
//			pthread_mutex_unlock(&mutex);
			break;
		}

		p[index] = normalMatch.front();
		normalMatch.pop_front();
		//说明该玩家 已下线 不匹配该玩家
		if(onlinePlayers.count(p[index].first) == 0) {
//			pthread_mutex_unlock(&mutex);
			continue;
		}
		//该玩家在线 但是 现在的玩家的登陆时间 和 当时匹配时 玩家登陆时间 不一样说明是 退出过 重登的
		if(onlinePlayers.count(p[index].first) != 0 && uidFd[p[index].first].second != p[index].second){
//			pthread_mutex_unlock(&mutex);
			continue;
		}
		index++;
		if(index == 2){
			index = 0;
			std::string uid = p[0].first;
			std::string anotherUid = p[1].first;

			//获取rid
			std::string rid = std::to_string(roomId);
			roomId++;

			//创建房间
			room *rm = new room(rid, 1);
			onlineRooms[rid] = rm;

			//将该玩家加入房间
			int fd = uidFd[uid].first;
			int anotherFd = uidFd[anotherUid].first;
			rm->addPlayer(fd, *onlinePlayers[uid], GoBang::BLACK);
			rm->addPlayer(anotherFd, *onlinePlayers[anotherUid], GoBang::WHITE);
			playerRoom[uid] = rm;
			playerRoom[anotherUid] = rm;

//			pthread_mutex_unlock(&mutex);

			GoBang::GoBangResponse goBangResponse;
			goBangResponse.set_type(GoBang::NORMAL_MATCH);
			GoBang::NormalMatchResponse *normalMatchResponse = goBangResponse.mutable_normalmatchresp();
			normalMatchResponse->set_rid(rid);
			normalMatchResponse->set_success(true);
			//序列化
			std::string r = goBangResponse.SerializeAsString();
			length = r.length();
			ptr = &length;
			memset(sendBuff, 0, sizeof (sendBuff));
			memcpy(sendBuff, ptr, sizeof (int));
			memcpy(sendBuff + 4, r.c_str(), length);
			//发送
			std::cout << "____________" << 4 << "  " << r.length() << '\n';
			send(fd,sendBuff,4 + r.length(),0);
			send(anotherFd,sendBuff,4 + r.length(),0);


//			pthread_mutex_lock(&mutex);
			onlineRooms[rid]->initGame();

			//发送开始游戏
			goBangResponse.set_type(GoBang::GAMESTART);
			GoBang::GameStart *gameStart = goBangResponse.mutable_gamestart();

			anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
			GoBang::PieceColor anotherC = onlineRooms[rid]->getAnotherPlayerPieceColor(uid);
			gameStart->set_mycolor(onlineRooms[rid]->getPlayerPieceColor(uid));
			gameStart->set_competitorcolor(anotherC);
			gameStart->set_competitorid(allFd[anotherFd]);
			gameStart->set_competitorname(onlinePlayers[allFd[anotherFd]]->name());
			gameStart->set_rid(rid);

//			pthread_mutex_unlock(&mutex);

			//序列化
			r = goBangResponse.SerializeAsString();

			length = r.length();
			memset(sendBuff, 0, sizeof (sendBuff));
			memcpy(sendBuff, ptr, sizeof (int));
			memcpy(sendBuff + 4, r.c_str(), length);
			//发送
			std::cout << "____________" << 4 << "  " << r.length() << '\n';
			send(fd,sendBuff,4 + r.length(),0);

//			pthread_mutex_lock(&mutex);
			//发送给另外一个玩家
			gameStart->set_mycolor(anotherC);
			gameStart->set_competitorcolor(onlineRooms[rid]->getPlayerPieceColor(uid));
			gameStart->set_competitorid(uid);
			gameStart->set_competitorname(onlinePlayers[allFd[fd]]->name());
			gameStart->set_rid(rid);
//			pthread_mutex_unlock(&mutex);

			//序列化
			r = goBangResponse.SerializeAsString();
			length = r.length();
			memset(sendBuff, 0, sizeof (sendBuff));
			memcpy(sendBuff, ptr, sizeof (int));
			memcpy(sendBuff + 4, r.c_str(), length);
			//发送
			send(anotherFd,sendBuff,4 + r.length(),0);


			//发送setpieceresponse
//			goBangResponse.clear_type();
//			goBangResponse.clear_gamestart();
			goBangResponse.set_type(GoBang::SET_PIECE);
			GoBang::SetPieceResponse *setPieceResponse = goBangResponse.mutable_setpieceresp();
			setPieceResponse->set_success(true);

//			pthread_mutex_lock(&mutex);

			setPieceResponse->set_next(onlineRooms[rid]->getNextColor());
			GoBang::Border *border = setPieceResponse->mutable_b();
			border->set_row_2(onlineRooms[rid]->getBorder().row_2());
			border->set_row_4(onlineRooms[rid]->getBorder().row_4());
			border->set_row_4(onlineRooms[rid]->getBorder().row_6());
			border->set_row_8(onlineRooms[rid]->getBorder().row_8());
			border->set_row_10(onlineRooms[rid]->getBorder().row_10());
			border->set_row_12(onlineRooms[rid]->getBorder().row_12());
			border->set_row_14(onlineRooms[rid]->getBorder().row_14());
			border->set_row_16(onlineRooms[rid]->getBorder().row_16());
//			(*border) = onlineRooms[rid]->getBorder();

//			pthread_mutex_unlock(&mutex);

			//序列化
			r = goBangResponse.SerializeAsString();
			length = r.length();
			ptr = &length;
			memset(sendBuff, 0, sizeof (sendBuff));
			memcpy(sendBuff, ptr, sizeof (int));
			memcpy(sendBuff + 4, r.c_str(), length);
			//发送
			std::cout << "____________" << 4 << "  " << r.length() << '\n';
			send(fd,sendBuff,4 + r.length(),0);
			send(anotherFd,sendBuff,4 + r.length(),0);

//			pthread_mutex_lock(&mutex);
			onlineRooms[rid]->gameState = true;
//			pthread_mutex_unlock(&mutex);
		}
		else{
//			pthread_mutex_unlock(&mutex);
		}
	}
	pthread_mutex_unlock(&mutex);
	return nullptr;
}

void* rankMatchThread(void *arg){
	int idx = *(int *)arg;
	delete (int *)arg;

	//uid time
	std::pair<std::string, int> p[2];
	int index = 0;
	int length = 0;
	int *ptr = &length;
	char sendBuff[4096];
	pthread_mutex_lock(&mutex);
	while(true){
//		pthread_mutex_lock(&mutex);
		if(rankMatch[idx].empty()){
			//凑不出两个人 将已近取出的一个人放回去
			if(index == 1) rankMatch[idx].push_front(p[index - 1]);
			break;
		}
		if(rankMatch[idx].size() == 1 && index == 0) {
//			pthread_mutex_unlock(&mutex);
			break;
		}

		p[index] = rankMatch[idx].front();
		rankMatch[idx].pop_front();
		//说明该玩家 已下线 不匹配该玩家
		if(onlinePlayers.count(p[index].first) == 0) {
//			pthread_mutex_unlock(&mutex);
			continue;
		}
		//该玩家在线 但是 现在的玩家的登陆时间 和 当时匹配时 玩家登陆时间 不一样说明是 退出过 重登的
		if(onlinePlayers.count(p[index].first) != 0 && uidFd[p[index].first].second != p[index].second){
//			pthread_mutex_unlock(&mutex);
			continue;
		}
		index++;
		if(index == 2){
			index = 0;
			std::string uid = p[0].first;
			std::string anotherUid = p[1].first;

			//获取rid
			std::string rid = std::to_string(roomId);
			roomId++;

			//创建房间
			room *rm = new room(rid, 2);
			onlineRooms[rid] = rm;

			//将该玩家加入房间
			int fd = uidFd[uid].first;
			int anotherFd = uidFd[anotherUid].first;
			rm->addPlayer(fd, *onlinePlayers[uid], GoBang::BLACK);
			rm->addPlayer(anotherFd, *onlinePlayers[anotherUid], GoBang::WHITE);
			playerRoom[uid] = rm;
			playerRoom[anotherUid] = rm;

//			pthread_mutex_unlock(&mutex);

			GoBang::GoBangResponse goBangResponse;
			goBangResponse.set_type(GoBang::RANK_MATCH);
			GoBang::RankMatchResponse *rankMatchResponse = goBangResponse.mutable_rankmatchresp();
			rankMatchResponse->set_rid(rid);
			rankMatchResponse->set_success(true);
			//序列化
			std::string r = goBangResponse.SerializeAsString();
			length = r.length();
			ptr = &length;
			memset(sendBuff, 0, sizeof (sendBuff));
			memcpy(sendBuff, ptr, sizeof (int));
			memcpy(sendBuff + 4, r.c_str(), length);
			//发送
			std::cout << "____________" << 4 << "  " << r.length() << '\n';
			send(fd,sendBuff,4 + r.length(),0);
			send(anotherFd,sendBuff,4 + r.length(),0);


//			pthread_mutex_lock(&mutex);
			onlineRooms[rid]->initGame();

			//发送开始游戏
			goBangResponse.set_type(GoBang::GAMESTART);
			GoBang::GameStart *gameStart = goBangResponse.mutable_gamestart();

			anotherFd = onlineRooms[rid]->getAnotherPlayerFd(uid);
			GoBang::PieceColor anotherC = onlineRooms[rid]->getAnotherPlayerPieceColor(uid);
			gameStart->set_mycolor(onlineRooms[rid]->getPlayerPieceColor(uid));
			gameStart->set_competitorcolor(anotherC);
			gameStart->set_competitorid(allFd[anotherFd]);
			gameStart->set_competitorname(onlinePlayers[allFd[anotherFd]]->name());
			gameStart->set_rid(rid);

//			pthread_mutex_unlock(&mutex);

			//序列化
			r = goBangResponse.SerializeAsString();

			length = r.length();
			memset(sendBuff, 0, sizeof (sendBuff));
			memcpy(sendBuff, ptr, sizeof (int));
			memcpy(sendBuff + 4, r.c_str(), length);
			//发送
			std::cout << "____________" << 4 << "  " << r.length() << '\n';
			send(fd,sendBuff,4 + r.length(),0);

//			pthread_mutex_lock(&mutex);
			//发送给另外一个玩家
			gameStart->set_mycolor(anotherC);
			gameStart->set_competitorcolor(onlineRooms[rid]->getPlayerPieceColor(uid));
			gameStart->set_competitorid(uid);
			gameStart->set_competitorname(onlinePlayers[allFd[fd]]->name());
			gameStart->set_rid(rid);
//			pthread_mutex_unlock(&mutex);

			//序列化
			r = goBangResponse.SerializeAsString();
			length = r.length();
			memset(sendBuff, 0, sizeof (sendBuff));
			memcpy(sendBuff, ptr, sizeof (int));
			memcpy(sendBuff + 4, r.c_str(), length);
			//发送
			send(anotherFd,sendBuff,4 + r.length(),0);


			//发送setpieceresponse
//			goBangResponse.clear_type();
//			goBangResponse.clear_gamestart();
			goBangResponse.set_type(GoBang::SET_PIECE);
			GoBang::SetPieceResponse *setPieceResponse = goBangResponse.mutable_setpieceresp();
			setPieceResponse->set_success(true);

//			pthread_mutex_lock(&mutex);

			setPieceResponse->set_next(onlineRooms[rid]->getNextColor());
			GoBang::Border *border = setPieceResponse->mutable_b();
			border->set_row_2(onlineRooms[rid]->getBorder().row_2());
			border->set_row_4(onlineRooms[rid]->getBorder().row_4());
			border->set_row_4(onlineRooms[rid]->getBorder().row_6());
			border->set_row_8(onlineRooms[rid]->getBorder().row_8());
			border->set_row_10(onlineRooms[rid]->getBorder().row_10());
			border->set_row_12(onlineRooms[rid]->getBorder().row_12());
			border->set_row_14(onlineRooms[rid]->getBorder().row_14());
			border->set_row_16(onlineRooms[rid]->getBorder().row_16());
//			(*border) = onlineRooms[rid]->getBorder();

//			pthread_mutex_unlock(&mutex);

			//序列化
			r = goBangResponse.SerializeAsString();
			length = r.length();
			ptr = &length;
			memset(sendBuff, 0, sizeof (sendBuff));
			memcpy(sendBuff, ptr, sizeof (int));
			memcpy(sendBuff + 4, r.c_str(), length);
			//发送
			std::cout << "____________" << 4 << "  " << r.length() << '\n';
			send(fd,sendBuff,4 + r.length(),0);
			send(anotherFd,sendBuff,4 + r.length(),0);

//			pthread_mutex_lock(&mutex);
			onlineRooms[rid]->gameState = true;
//			pthread_mutex_unlock(&mutex);
		}
		else{
//			pthread_mutex_unlock(&mutex);
		}
	}
	pthread_mutex_unlock(&mutex);
	return nullptr;
}


void initRList(){
	//设置排名缓存 查询前100名
	UserDao userDao;
	RList = userDao.querySomeUser(connectionPool, 100, 10000);
}

bool compare(GoBang::Player &x, GoBang::Player &y){
	return x.points() > y.points();
}

void updateList(const GoBang::Player p){
	bool flag = false;
	for(auto &it : RList){
		if(it.uid() == p.uid()){
			it.set_points(p.points());
			flag = true;
			break;
		}
	}

	if(flag == false){
		//不再当前 前100名中
		if(RList.size() < 100){
			//前100名没有满
			RList.push_back(p);
		}
		else if(RList.size() == 100){
			//前100名满了 和 分数最小的比较 如果超过了 将它替换下来
			if(RList.back().points() < p.points()){
				RList.pop_back();
				RList.push_back(p);
			}
		}

	}
	std::sort(RList.begin(), RList.end(), compare);
}