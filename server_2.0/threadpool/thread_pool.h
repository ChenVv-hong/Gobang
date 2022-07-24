//
// Created by chen on 2022/6/17.
//

#ifndef TEST_THREAD_POOL_H
#define TEST_THREAD_POOL_H
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <cerrno>
#include <queue>
#include <vector>

#define THREAD_INCREMENT 10     //线程扩容或销毁 一次的增量或减量
#define ADMIN_INTERVAL 3       //管理者线程 每隔多少秒 处理一次
typedef void* (*task_func)(void *arg);

struct task
{
	task_func func;
	void *arg;
};
void* thread_func(void *arg);
void* admin_thread_func(void *arg);

class thread_pool{
public:
	thread_pool(int min_thr, int max_thr, int max_tasks, bool is_dynamic = false);
	~thread_pool();
	thread_pool(const thread_pool& a) = delete;
	thread_pool& operator= (const thread_pool& a) = delete;
	void close();        //关闭线程池
	void add_task(task t);  //向任务队列添加任务
	void print_msg();       //打印当前线程池 工作状况
private:
	void init_mutex_cond();         //初始化 用到的条件变量和互斥量
	void destroy_mutex_cond();      //销毁 用到的条件变量和互斥量
	bool thread_is_alive(pthread_t);//判断线程是否存活
	friend void* thread_func(void *arg);        //工作线程回调函数
	friend void* admin_thread_func(void *arg);  //管理者线程回调函数
private:
	std::vector<pthread_t> threads; //线程池中的线程
	pthread_t admin_thr;            //管理者线程，用于管理线程池中的线程

	pthread_cond_t is_take; //条件变量 人物队列 不为空    空了 处理任务线程阻塞
	pthread_cond_t is_push; //条件变量 人物队列 不为满    满了 添加任务阻塞

	pthread_mutex_t queue_lock;         //用于线程池的互斥锁
	pthread_mutex_t thread_counter_lock;//用于busy_thr_num的互斥锁

	//下面三个变量 为初始化赋值的 后续不会再更改 为只读 不需要加锁
	int min_thr_num;    //最小线程数
	int max_thr_num;    //最大线程数
	int max_tasks_num;          //最大任务队列

	int busy_thr_num;   //忙状态线程数

	int live_thr_num;   //存活线程数
	int destroy_thr_num;//要销毁线程数
//	int tasks_num;      //任务队列中的任务数量
	bool shutdown;      //线程池是否打开
	std::queue<task> task_q;    //任务队列

	bool is_dynamic;
};


#endif //TEST_THREAD_POOL_H
