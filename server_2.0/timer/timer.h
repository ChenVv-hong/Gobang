//
// Created by chen on 2022/6/25.
//

#ifndef MYWEBSERVER_TIMER_H
#define MYWEBSERVER_TIMER_H
#include <time.h>

#define TIMESLOT 5
class timer;
struct client_data{
	int sock_fd;
	int ep_fd;
	timer * t;
};

struct timer{
	time_t expire;
	void (*cb_func)(client_data *);
	client_data *user_data = nullptr;
	timer *next = nullptr;
	timer *prev = nullptr;
};

class sort_time_list{
public:
	sort_time_list():head(nullptr),tail(nullptr){};
	sort_time_list(const sort_time_list&) = delete;
	sort_time_list& operator=(const sort_time_list &) = delete;
	~sort_time_list();
	/*
	 * 添加定时器
	 */
	void addTimer(timer *);
	/*
	 * 当某个定时任务发生变化时 需要调整。 要求被调整的定时器在 链表中
	 */
	void adjustTimer(timer *);
	/*
	 * 删除定时器 要求 被删除的定时器 在链表中
	 */
	void delTimer(timer *);
	/*
	 * 处理链表中的定时任务
	 */
	void tick();

private:
	/*
	 * 重载的辅助函数 在链表lst中 找地方插入
	 */
	void addTimer(timer *t, timer *lst);
private:
	timer *head;
	timer *tail;
};


#endif //MYWEBSERVER_TIMER_H
