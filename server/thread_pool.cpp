//
// Created by chen on 2022/6/17.
//
#include "thread_pool.h"

//构造函数
thread_pool::thread_pool(int min_thr, int max_thr, int max_tasks, bool is_d){
	//参数检查
	if(min_thr <= 0) min_thr = 3;
	if(max_thr < min_thr) max_thr = min_thr + 50;
	if(max_tasks < 1000) max_tasks = 1000;
	//初始化
	this->min_thr_num = min_thr;
	this->max_thr_num = max_thr;
	this->max_tasks_num = max_tasks;
	this->is_dynamic = is_d;

	this->busy_thr_num = 0;
	this->live_thr_num = min_thr;
	this->destroy_thr_num = 0;
	this->shutdown = false;
//	this->tasks_num = 0;
	//初始化 mutex 和 cond
	this->init_mutex_cond();

	//工作线程创建
	threads.resize(max_thr_num, 0);
	for(int i = 0; i < this->min_thr_num; i++){
		pthread_create(&this->threads[i], nullptr, (thread_func), (void *)this);
//		std::cout << "work_thread " << this->threads[i] << " has created !" << '\n';
	}
	if(is_d){
		//管理者线程创建
		pthread_create(&this->admin_thr, nullptr, admin_thread_func, (void *)this);
		//	std::cout << "admin_thread " << this->admin_thr << " has created !" << '\n';
	}

}
//析构函数
thread_pool::~thread_pool(){
	pthread_mutex_lock(&this->queue_lock);
	bool t = this->shutdown;
	pthread_mutex_unlock(&this->queue_lock);
	if(t == false){
		this->close();
	}
}
//初始化 mutex 和 cond
void thread_pool::init_mutex_cond(){
	//TODO 返回值检查
	pthread_mutex_init(&(this->queue_lock), nullptr);
	pthread_mutex_init(&(this->thread_counter_lock), nullptr);
	pthread_cond_init(&(this->is_push), nullptr);
	pthread_cond_init(&(this->is_take), nullptr);
}
//销毁 mutex 和 cond
void thread_pool::destroy_mutex_cond(){
	//TODO 返回值检查
	pthread_mutex_destroy(&(this->queue_lock));
	pthread_mutex_destroy(&(this->thread_counter_lock));
	pthread_cond_destroy(&(this->is_push));
	pthread_cond_destroy(&(this->is_take));
}
//判断线程 是否存活
bool thread_pool::thread_is_alive(pthread_t tid){
	int ret = pthread_kill(tid, 0);
	if(ret == ESRCH){
		return false;
	}
	return true;
}

//添加任务
void thread_pool::add_task(task t){
	pthread_mutex_lock(&this->queue_lock);
	while(this->task_q.size() == this->max_tasks_num && !this->shutdown){
		pthread_cond_wait(&this->is_push,&this->queue_lock);
	}
	if(this->shutdown){
		std::cout << "线程池已关闭 无法添加任务" << '\n';
		pthread_mutex_unlock(&this->queue_lock);
		return;
	}

	//添加任务
	this->task_q.push(t);
	//唤醒 空闲线程 即阻塞在 is_take上的线程
	pthread_cond_signal(&this->is_take);
	pthread_mutex_unlock(&this->queue_lock);

}
//线程池关闭
void thread_pool::close(){
	//TODO 关闭 管理线程 停止添加任务 等待任务队列中的 所有任务完成后关闭所有工作线程
	pthread_mutex_lock(&this->queue_lock);
	this->shutdown = true;
	pthread_mutex_unlock(&this->queue_lock);
	//此处再次唤醒 是防止 还有空闲线程阻塞在 is_take上 导致后续回收线程 join时 一直阻塞 回收不了
	for(int i = 0; i < this->live_thr_num; i++){
		pthread_cond_broadcast(&this->is_take);
	}
	//回收 所有线程
	if(this->is_dynamic) pthread_join(this->admin_thr, nullptr);

	for(int i = 0; i < this -> max_thr_num ; i++){
		if(this->threads[i] != 0 && this->thread_is_alive(this->threads[i])){
			pthread_join(this->threads[i], nullptr);
		}
	}
	this->destroy_mutex_cond();
}
//打印线程池信息
void thread_pool::print_msg(){
	int min_thr;    //最小线程数
	int max_thr;    //最大线程数
	int busy_thr;   //忙状态线程数
	int live_thr;   //存活线程数
	int tasks_n;  //任务数量
	pthread_mutex_lock(&this->thread_counter_lock);
	busy_thr = this->busy_thr_num;
	pthread_mutex_unlock(&this->thread_counter_lock);
	pthread_mutex_lock(&this->queue_lock);
	live_thr = this->live_thr_num;
	tasks_n = this->task_q.size();
	pthread_mutex_unlock(&this->queue_lock);
	std::cout << "最小线程数: " << this->min_thr_num << '\n';
	std::cout << "最大线程数: " << this->max_thr_num << '\n';
	std::cout << "当前忙状态线程数: " << busy_thr << '\n';
	std::cout << "当前存活状态线程数: " << live_thr << '\n';
	std::cout << "当前任务数量: " << tasks_n << '\n';
}

//工作线程
void* thread_func(void *arg){
	//向任务队列获取任务 执行任务
	thread_pool *pool = (thread_pool *)arg;
	while(true){
		pthread_mutex_lock(&pool->queue_lock);
		//当任务队列为空 阻塞 等待唤醒
		while(pool->task_q.empty() && !pool->shutdown){
			//等待 任务队列中有任务 换而言之 当前这个线程是空闲的
			pthread_cond_wait(&pool->is_take,&pool->queue_lock);
			//如果有线程需要被销毁
			if(pool->destroy_thr_num > 0){
				pool->destroy_thr_num --;
				//当前存活线程数 大于 最小线程数  那就释放该线程
				if(pool->live_thr_num > pool->min_thr_num){
					pool->live_thr_num--;
					pthread_mutex_unlock(&pool->queue_lock);
					pthread_exit(nullptr);
				}
			}
		}
		//当线程池关闭 并且 任务队列 为空
		if(pool->task_q.empty() && pool->shutdown){
			//当线程池关闭 并且任务队列中任务全部完成 释放线程
			pool->live_thr_num--;
			pthread_mutex_unlock(&pool->queue_lock);
			pthread_exit(nullptr);
		}
		//只要 队列中有任务 就往下执行 即使 线程池关闭 也要把任务队列中的任务处理完
		//取出任务
		task t = pool->task_q.front();
		pool->task_q.pop();
		pthread_cond_signal(&pool->is_push);    //通知可以 放任务
		pthread_mutex_unlock(&pool->queue_lock);        //取出任务后 迅速 解锁

		//即将执行任务
		pthread_mutex_lock(&pool->thread_counter_lock);
		pool->busy_thr_num++;                           //忙状态线程数 ++
		pthread_mutex_unlock(&pool->thread_counter_lock);
//		std::cout << pthread_self() << "开始执行任务！\n";
		//执行任务
		t.func(t.arg);
//		std::cout << pthread_self() << "任务完成！\n";
		//任务完成
		pthread_mutex_lock(&pool->thread_counter_lock);
		pool->busy_thr_num--;                           //忙状态线程数 --
		pthread_mutex_unlock(&pool->thread_counter_lock);

	}

}

//管理者线程
void* admin_thread_func(void *arg){
	//管理 线程池 扩容 销毁线程
	thread_pool *pool = (thread_pool *)arg;
	while(true){
		sleep(ADMIN_INTERVAL);

		pthread_mutex_lock(&pool->queue_lock);
		int live_thr_num = pool->live_thr_num;  //存活线程数
		int tasks_num = pool->task_q.size();    //任务数量
		if(pool->shutdown == true){
			pthread_mutex_unlock(&pool->queue_lock);
			break;
		}
		pthread_mutex_unlock(&pool->queue_lock);

		pthread_mutex_lock(&pool->thread_counter_lock);
		int busy_thr_num =   pool->busy_thr_num;        //忙状态线程数
		pthread_mutex_unlock(&pool->thread_counter_lock);

		//如果 任务数量大于存活线程数 并且 存活线程数 小于 最大线程数 时 扩容
		if(tasks_num > live_thr_num && live_thr_num < pool->max_tasks_num){
			std::cout << "需要扩容线程\n";
			pthread_mutex_lock(&pool->queue_lock);
			for(int i = 0, j = 0; j < THREAD_INCREMENT && i < pool->max_thr_num  && live_thr_num < pool->max_tasks_num; i++){
				if(pool->threads[i] == 0 || !pool->thread_is_alive(pool->threads[i])){
					pthread_create(&pool->threads[i], nullptr, thread_func, (void *)pool);
//					std::cout << "work_thread " << pool->threads[i] << " has created !" << '\n';
					j++;
					pool->live_thr_num++;  //存活线程数++
				}
			}
			pthread_mutex_unlock(&pool->queue_lock);
		}

		//如果忙线程*2 小于 存活的线程数 并且 存活的线程数 大于 最小线程 时 销毁
		if(busy_thr_num * 2 < live_thr_num && live_thr_num > pool->min_thr_num){
//			std::cout << "需要销毁线程\n";
			pthread_mutex_lock(&pool->queue_lock);
			pool->destroy_thr_num = THREAD_INCREMENT;
			pthread_mutex_unlock(&pool->queue_lock);
			for(int i = 0; i < THREAD_INCREMENT; i++){
				//通知在空闲状态的线程 他们会自行销毁
				pthread_cond_signal(&pool->is_take);
			}
		}

	}
	pthread_exit(nullptr);
}