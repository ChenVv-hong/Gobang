#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <vector>
#include <unordered_map>
#include <list>

#include "thread_pool.h"
#include "room.h"
#include "player.h"


#define SRV_PORT 9999
#define MAX_CLIENT  1024        //最大连接数
#define MAX_BUFFER_SIZE    4096        //缓冲区大小
#define LISTEN_QU   128         //listen监听队列大小

#define CONNECT 110            //匹配请求
#define CONNECT_SUCCESS 120	   //通常为服务器返回数据 表示连接/匹配成功
#define CONNECT_FAIL 130	   //通常为服务器返回数据 表示连接/匹配失败

#define SETPIECE 140           //落子信息
//既可以客户端发送数据 也可以为客户端接收

#define UNDO 150               //悔棋请求/悔棋确认
#define UNDO_YES 151           //悔棋同意
#define UNDO_NO 152            //悔棋拒绝

#define GAMEOVER 160           //游戏结束
#define DISCONNECT 170         //意外断开连接
#define SURRENDER 190          //投降

#define TIE 200               //求和请求/求和确认
#define TIE_YES 201           //求和同意
#define TIE_NO 202            //求和拒绝

#define MSG 300               //消息
#define RESTART 400           //重开
#define CREATE_ROOM 500       //创建房间
#define JOIN_ROOM   501       //加入房间
#define JOIN_ROOM_FAIL   502       //加入房间失败
#define CREATE_ROOM_SUCCESS 503       //创建房间

#define BLACK_PIECE 0
#define WHITE_PIECE 1
#define NO_PIECE 2


struct task_arg{
	int sockfd;
	int epfd;
};
void print_sys_error(const char * s);
int init_socket();
void* task_process(void *arg);
std::vector<std::string> split(const char *s,const char ch);
void delete_key(std::unordered_map<int, room> &, int);
void delete_key(std::unordered_map<int, player> &, int);
void delete_key(std::unordered_map<std::string , room> &, std::string &);
void *matched_thr_func(void *arg);

pthread_mutex_t mutex;
std::list<player> players;  //在线玩家
std::list<room> rooms;      //正在运行的房间
std::unordered_map<int, room> mp_fd_r;      // fd -> room 的映射
std::unordered_map<int, player> mp_fd_p;    //fd -> player 的映射
std::unordered_map<std::string , room> mp_id_r; //room_id -> room 的映射
std::queue<player> matched_pool;    //匹配队列
static int room_id = 1;

int main(){
	
	//初始化 线程池
	thread_pool pool(6, 20, 10000);
	pthread_t matched_thr;
	pthread_mutex_init(&mutex, nullptr);
	pthread_create(&matched_thr, nullptr, matched_thr_func, nullptr);
	int ret;
	int lfd = init_socket();

	//创建epoll
	int epfd = epoll_create(MAX_CLIENT);
	//将lfd加入 epoll
	epoll_event e, events[MAX_CLIENT];
	e.data.fd = lfd;
	e.events = EPOLLIN;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &e);
	if(ret == -1) print_sys_error("main 1 epoll_ctl error !");

	//将标准输入设置非阻塞
	int flag = fcntl(STDIN_FILENO, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(STDIN_FILENO, F_SETFL, flag);

	char buf[MAX_BUFFER_SIZE];
	memset(buf, 0, sizeof(buf));
	std::cout << "Gobang server has started.....\n";
	while(true){
		int n = read(STDIN_FILENO, buf, sizeof buf);
		//n == -1 可能是没读到数据 也可能是 错误
		if(n > 0 && strcmp(buf, "quit\n") == 0){
			break;
			//TODO 服务端 退出 尽量做安全退出
		}
		if(n > 0 && strcmp(buf, "cat\n") == 0){
			//输入 cat 查看 线程池 工作状态
//			pool.print_msg();
			for(auto it : mp_fd_r){
				std::cout << it.first << ' ' << it.second.getRoomId() << '\n';
			}
			for(auto it : mp_fd_p){
				std::cout << it.first << ' ' << it.second.getNickname() << '\n';
			}
		}
		int nready = epoll_wait(epfd, events, MAX_CLIENT, 0);
		if(nready == 0) continue;
//		std::cout << "nready: " << nready << '\n';
		for(int i = 0; i < nready; i++){
			if(events[i].data.fd == lfd){
				//监听 fd用来 处理连接
				sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);
				while(true){
					ret = accept(events[i].data.fd, (sockaddr *)&client_addr, &client_len);
					if(ret == -1){
						if(errno == EINTR) continue;        //被中断 再次连接
						else{
							print_sys_error("accept error");
						}
					}else break;
				}
				//连接成功 打印客户端 连接信息
				char client_ip[INET_ADDRSTRLEN];
				int client_port = ntohs(client_addr.sin_port);
				inet_ntop(AF_INET,&client_addr.sin_addr.s_addr, client_ip, INET_ADDRSTRLEN);
				std::cout << "ip: " << client_ip << " port: " << client_port << " has connected\n";

				/*
				 * 如在EPOLLIN中读出数据后。对于整个工作流程来说，读出数据只是工作的一半，还需要将处理后的数据写到对端，
				 * 但由于注册了EPOLLONESHOT，EPOLLIN的事件被唤醒后该fd已经从epollfd中删去，所以对于尚未完成的工作，要重新注册，
				 * 比如这里，重新注册fd为EPOLL|EPOLLET|EPOLLONESHOT。
				 * EPOLLONESHOT是必须的，因为要保证向对端写入时只有一个线程在该fd上工作。
				 */

				//加入监听红黑树中
				epoll_event temp;
				temp.data.fd = ret;
				temp.events = EPOLLIN | EPOLLONESHOT;
				ret = epoll_ctl(epfd, EPOLL_CTL_ADD, ret, &temp);
				if(ret == -1) print_sys_error("main 2 epoll_ctl error");

			}else{
				//向任务队列中 添加任务
				pool.add_task({task_process, new task_arg{events[i].data.fd, epfd}});
//				pool.print_msg();
			}
		}
	}
	//服务端 安全退出了
	pthread_cancel(matched_thr);
	pthread_join(matched_thr, nullptr);
	pthread_mutex_destroy(&mutex);
	pool.close();
	return 0;
}


void print_sys_error(const char * s){
	perror(s);
	exit(1);
}

int init_socket(){
	int ret;
	int lfd = socket(PF_INET, SOCK_STREAM, 0);
	if(lfd == -1) print_sys_error("socket error!");

	struct sockaddr_in addr;
	addr.sin_port = htons(SRV_PORT);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(lfd, (sockaddr *)&addr, sizeof (addr));
	if(ret == -1) print_sys_error("bind error!");

	ret = listen(lfd, LISTEN_QU);
	if(ret == -1) print_sys_error("listen error!");
	return lfd;
}

void* task_process(void *arg){
	pthread_mutex_lock(&mutex);
	//获取参数 ，获取完后 立马释放内存
	task_arg *a = (task_arg *)arg;
	int sockfd = a->sockfd;
	int epfd = a->epfd;
	delete a;

	int ret;
	char buf[MAX_BUFFER_SIZE];
	memset(buf, 0, sizeof buf);
	int cnt;
	cnt = read(sockfd, buf, sizeof(buf));
//	write(STDOUT_FILENO, buf, cnt);
	if(cnt == 0){
		//TODO 向另一个客户端发送对方断开连接消息
		//读到的字节数为0 说明对端已经关闭
		//从监听红黑树中去除 sockfd
		ret = epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, nullptr);
		if(ret == -1) print_sys_error("task_process 1 epoll_ctl error");

		//获取房间 和 房间中的player
		if(mp_fd_p.count(sockfd) == 0){
			std::cout << "error_4\n";
			close(sockfd);
			pthread_mutex_unlock(&mutex);
			return nullptr;
		}
		player p = mp_fd_p[sockfd];
		if(mp_fd_r.count(sockfd) == 0) {
			//这位玩家 不再房间中 即这个玩家 还在匹配
			//只需要删除 玩家信息
			players.remove(p);
			//删除 玩家映射
			delete_key(mp_fd_p, sockfd);
		}else{
			//这位玩家在房间中

			room r = mp_fd_r[sockfd];
			player ano_p;
			if(!r.anotherPlayer(p, ano_p)){
				std::cout << "error_3\n";
				pthread_mutex_unlock(&mutex);
				return nullptr;
			}
			//TODO 是否有ano_p的映射
			if(ano_p.getFd() != -1){
				//房间中有两位 玩家
				//向另外一位玩家 发送消息
				std::string msg = std::to_string(DISCONNECT) + '\n';
				write(ano_p.getFd(), msg.c_str(), msg.length());
				//删除 断开连接的玩家
				players.remove(p);
				//删除 玩家映射
				delete_key(mp_fd_p, sockfd);
				//删除 房间
				rooms.remove(r);
				//删除房间映射
				delete_key(mp_fd_r, sockfd);
				delete_key(mp_fd_r, ano_p.getFd());
				delete_key(mp_id_r, r.room_id);
			}
			else{
				//房间中 只有一位玩家
				//删除 断开连接的玩家
				players.remove(p);
				//删除 玩家映射
				delete_key(mp_fd_p, sockfd);
				//删除 房间
				rooms.remove(r);
				//删除房间映射
				delete_key(mp_fd_r, sockfd);
				delete_key(mp_id_r, r.room_id);
			}

		}
		std::cout << "someone disconnected\n";
		close(sockfd);
	}
	else if(cnt > 0){

		player p,ano_p;
		room r;
		//客户端发送数据 进行处理
		std::vector<std::string> msg = split(buf, '\n');
		for(int i = 0; i < msg.size(); i++) std::cout << i <<":" << msg[i] << ' ';
		std::cout << '\n';
		int index = 0;
		while(index < msg.size()){
			//如果是匹配请求 那么玩家 和 房间 都没创建 要加判断
			int code = atoi(msg[index].c_str());
			if(code != CONNECT && code != CREATE_ROOM && code != JOIN_ROOM){
				//获取房间 和 房间中的player
				if(mp_fd_p.count(sockfd) == 0){
					std::cout << "error_1\n";
					pthread_mutex_unlock(&mutex);
					return nullptr;
				}
				p = mp_fd_p[sockfd];
				if(mp_fd_r.count(sockfd) == 0) {
					std::cout << "error_2\n";
					pthread_mutex_unlock(&mutex);
					return nullptr;
				}
				r = mp_fd_r[sockfd];
				if(!r.anotherPlayer(p, ano_p)){
					std::cout << "error_3\n";
					pthread_mutex_unlock(&mutex);
					return nullptr;
				}
			}
			switch (code) {
				case(CONNECT):{
//				std::cout << "CONNECT\n";
					//TODO 互斥
					//匹配 请求 将 玩家 加入到匹配队列
					if(mp_fd_p.count(sockfd)){
						//说明 该玩家 已近是在线玩家 直接 添加到匹配队列
//					pthread_mutex_lock(&mutex);
						matched_pool.push(mp_fd_p[sockfd]);
//					pthread_mutex_unlock(&mutex);
					}else{
						//说明 该玩家 没有登入过 为他创建角色 并且添加到匹配队列
						p.setFd(sockfd);
						p.setNickname(msg[index + 1]);
						players.push_back(p);
						//建立映射
						mp_fd_p[sockfd] = p;
//					std::cout << sockfd << ' ' << "p\n";

//					pthread_mutex_lock(&mutex);
						matched_pool.push(p);
//					pthread_mutex_unlock(&mutex);
					}
					index += 2;
					break;
				}
				case(UNDO):{
					if(r.mask[0]){
						//说明该房间中有玩家 已近发送了 悔棋请求 那么另一个人的发的不做响应
						index++;
						break;
					}
					//直接转发给客户端
					std::string ss = msg[index] + '\n';
					write(ano_p.getFd(), ss.c_str(), ss.length());
					r.mask[0] = true;
					index++;
					break;
				}
				case(TIE):{

					if(r.mask[1]){
						//说明该房间中有玩家 已近发送了 和棋请求 那么另一个人的发的不做响应
						index++;
						break;
					}
					//直接转发给客户端
					std::string ss = msg[index] + '\n';
					write(ano_p.getFd(), ss.c_str(), ss.length());
					r.mask[1] = true;
					index++;
					break;
				}
				case(SURRENDER):{

					if(r.mask[2]){
						//说明该房间中有玩家 已近发送了 投降 那么另一个人发的不做响应
						index++;
						break;
					}
					std::string ss = msg[index] + '\n';
					write(ano_p.getFd(), ss.c_str(), ss.length());
					r.mask[2] = true;

					//TODO 加互斥
					//投降了 删除该房间
					rooms.remove(r);
					//删除房间映射
					delete_key(mp_fd_r, sockfd);
					delete_key(mp_fd_r, ano_p.getFd());
					//删除房间id
					delete_key(mp_id_r, r.room_id);
					index++;
					break;
				}
				case(UNDO_YES):{

					//直接转发给客户端
					std::string ss = msg[index] + '\n';
					write(ano_p.getFd(), ss.c_str(), ss.length());
					//请求已被处理
					r.mask[0] = false;
					index++;
					break;
				}
				case(UNDO_NO):{

					//直接转发给客户端
					std::string ss = msg[index] + '\n';
					write(ano_p.getFd(), ss.c_str(), ss.length());
					r.mask[0] = false;
					index++;
					break;
				}
				case(TIE_YES):{

					//直接转发给客户端
					std::string ss = msg[index] + '\n';
					write(ano_p.getFd(), ss.c_str(), ss.length());
					r.mask[1] = false;

					//游戏结束了 删除该房间
					rooms.remove(r);
					//删除房间映射
					delete_key(mp_fd_r, sockfd);
					delete_key(mp_fd_r, ano_p.getFd());
					delete_key(mp_id_r, r.room_id);
					index++;
					break;
				}
				case(TIE_NO):{

					//直接转发给客户端
					std::string ss = msg[index] + '\n';
					write(ano_p.getFd(), ss.c_str(), ss.length());
					r.mask[1] = false;
					index++;
					break;
				}
				case(SETPIECE):{
					std::string ss = msg[index] + '\n' + msg[index + 1] + '\n' + msg[index + 2] + '\n' + msg[index + 3] + '\n';
					write(ano_p.getFd(), ss.c_str(), ss.length());
					index += 4;
					break;
				}
				case(MSG):{

					//这些类型的数据 直接转发给客户端
					std::string ss = msg[index] + '\n' + msg[index + 1] + '\n';
					write(ano_p.getFd(), ss.c_str(), ss.length());
					index += 2;
					break;
				}
				case(GAMEOVER):{
					//这些类型的数据 直接转发给客户端
					//TODO
//				std::cout << "GAMEOVER\n";
					//TODO
					std::string ss = msg[index] + '\n' + msg[index + 1]; + '\n';
					write(ano_p.getFd(), ss.c_str(), cnt);
					//游戏结束了 删除该房间
					rooms.remove(r);
					//删除房间映射
					delete_key(mp_fd_r, sockfd);
					delete_key(mp_fd_r, ano_p.getFd());
					delete_key(mp_id_r, r.room_id);
					index += 2;
					break;
				}
				case(CREATE_ROOM):{
					std::cout << "create room\n";
					//注册玩家
					p.setFd(sockfd);
					p.setNickname(msg[index + 1]);
					players.push_back(p);
					//添加 映射
					mp_fd_p[sockfd] = p;
					//创建房间  暂时还没有另外一个玩家在房间 我们先把他创建着 令 fd = -1 方便 后面检查
					r.setP1(p);
					player ano_p;
					ano_p.setFd(-1);
					ano_p.setNickname("");
					r.setP2(ano_p);
					r.setRoomId(std::to_string(room_id++));
					//先不添加房间 等另外玩家加入 在添加
//				rooms.push_back(r);
					//添加房间 映射
					mp_fd_r[sockfd] = r;
					mp_id_r[r.getRoomId()] = r;
					std::string s = std::to_string(CREATE_ROOM_SUCCESS) + '\n' + r.getRoomId() + '\n';
//				std::cout << s;
					write(sockfd, s.c_str(), s.length());
					index += 2;
					break;
				}
				case(JOIN_ROOM):{
					std::cout << "join room\n";
					if(mp_id_r.count(msg[index + 1]) == 0){
						//说明房间号不存在
						std::string s = std::to_string(JOIN_ROOM_FAIL) + '\n';
						write(sockfd, s.c_str(), s.length());
						index += 3;
						break;
					}
					//获取 room
					r = mp_id_r[msg[index + 1]];
					//填充room
					if(r.p2.getFd() == -1){
						r.p2.setFd(sockfd);
						r.p2.setNickname(msg[index + 2]);
						players.push_back(r.p2);

						mp_fd_p[sockfd] = r.p2;
						rooms.push_back(r);

						//重新映射
						mp_fd_r[sockfd] = r;
						mp_fd_r[r.p1.getFd()] = r;

					}
					std::string s = std::to_string(CONNECT_SUCCESS) + '\n' + r.p2.getNickname() + '\n' + std::to_string(BLACK_PIECE) + '\n' + r.getRoomId() +'\n';
					write(r.p1.getFd(), s.c_str(), s.length());
					s.clear();
					s = std::to_string(CONNECT_SUCCESS) + '\n' + r.p1.getNickname() + '\n' + std::to_string(WHITE_PIECE) + '\n'+ r.getRoomId() +'\n';
					write(r.p2.getFd(), s.c_str(), s.length());
					index += 3;
				}
			}
		}

		//将监听的fd重新注册监听时间 应为设置了 EPOLLONESHOT
		epoll_event e;
		e.data.fd = sockfd;
		e.events = EPOLLIN | EPOLLONESHOT;
		epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &e);
	}else{
		//出错 摘除结点
		ret = epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, nullptr);
		if(ret == -1) print_sys_error("task_process 2 epoll_ctl error");
		close(sockfd);
	}
	pthread_mutex_unlock(&mutex);
	return nullptr;
}

std::vector<std::string> split(const char *s,const char ch){
	std::vector<std::string> ret;
	std::string temp("");
	for(int i = 0; i < strlen(s); i++){
		if(s[i] == ch){
			ret.push_back(temp);
			temp.clear();
		}else{
			temp += s[i];
		}
	}
	if(temp != "") ret.push_back(temp);
	return ret;
}

void delete_key(std::unordered_map<int, room>& map, int k) {
	std::unordered_map<int, room>::iterator it;
	int i_Total = 0;
	for (it = map.begin(); it != map.end();) {
		if (it->first == k) {
			//再删除迭代
			map.erase(it++);
			++i_Total;
		} else {
			it++;
		}
	}
}

void delete_key(std::unordered_map<int, player>& map, int k){
	std::unordered_map<int, player>::iterator it;
	int i_Total = 0;
	for (it = map.begin(); it != map.end();) {
		if (it->first == k) {
			//删除迭代
			map.erase(it++);
			++i_Total;
		} else {
			it++;
		}
	}
}

void delete_key(std::unordered_map<std::string , room> & map, std::string &k){
	std::unordered_map<std::string , room>::iterator it;
	int i_Total = 0;
	for (it = map.begin(); it != map.end();) {
		if (it->first == k) {
			//删除迭代
			map.erase(it++);
			++i_Total;
		} else {
			it++;
		}
	}
}

void *matched_thr_func(void *arg){
	player p[2];
	int index = 0;
	while(true){
//		std::cout << '\n';
		pthread_testcancel();
		pthread_mutex_lock(&mutex);
		if(matched_pool.size() >= 1){
//			std::cout << "aaaaa\n";
			p[index] = matched_pool.front();
			matched_pool.pop();
			//说明该玩家 已下线 不匹配该玩家
			if(mp_fd_p.count(p[index].getFd()) == 0) continue;
			index++;
		}
		if(index == 2){
//			std::cout << "bbbbbb\n";
			index = 0;
			room r(std::to_string(room_id), p[0], p[1]);
			rooms.push_back(r);
			//建立映射
			mp_fd_r[p[0].getFd()] = r;
			mp_fd_r[p[1].getFd()] = r;

			//发送匹配成功消息
			std::string s = std::to_string(CONNECT_SUCCESS) + '\n' + p[0].getNickname() + '\n' + std::to_string(BLACK_PIECE) + '\n' + std::to_string(room_id) +'\n';
			write(p[1].getFd(), s.c_str(), s.length());
			s.clear();
			s = std::to_string(CONNECT_SUCCESS) + '\n' + p[1].getNickname() + '\n' + std::to_string(WHITE_PIECE) + '\n'+ std::to_string(room_id++) +'\n';
			write(p[0].getFd(), s.c_str(), s.length());
		}
		pthread_mutex_unlock(&mutex);

	}
}
