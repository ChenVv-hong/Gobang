//
// Created by chen on 2022/6/25.
//

#include "timer.h"

sort_time_list::~sort_time_list() {
	while(this->head != nullptr){
		timer *temp = head;
		this->head = this->head->next;
		delete temp;
	}
}

void sort_time_list::addTimer(timer *t) {
	if(!t) return;
	if(!head){
		//链表中没有 节点 此节点充当头节点
		head = tail = t;
		return;
	}
	if(t->expire < head->expire){
		//如果插入节点的时间 小于头节点 那么插入在 头节点之前
		t->next = head;
		head->prev = t;
		head = t;
		return;
	}
	addTimer(t, head);
}

void sort_time_list::adjustTimer(timer *t) {
	if(!t) return;
	timer *next = t->next;
	//如果 节点t 是链表尾部结点 或者 他的下一个节点的时间 大于 他 那么不用调整
	if(!next || next->expire > t->expire){
		return;
	}
	//如果 t 是头节点 那么把他取出 重置头节点 然后放入 正确位置
	if(t == head){
		head = head->next;
		head->prev = nullptr;
		t->next = nullptr;
		addTimer(t, head);
	}
	else{
		//如果 t 不是 头节点
		timer *prev = t->prev;
		prev->next = next;
		next->prev = prev;
		t->prev = nullptr;
		t->next = nullptr;
		addTimer(t, next);
	}

}

void sort_time_list::delTimer(timer *t) {
	if(!t) return;
	//当链表中只有一个节点时
	if(t == head && t == tail){
		delete t;
		head = nullptr;
		tail = nullptr;
		return;
	}
	//当要删除的节点是头节点
	if(t == head){
		head = head->next;
		head->prev = nullptr;
		delete t;
		return;
	}
	//当要删除的节点是尾节点
	if(t == tail){
		tail = tail->prev;
		tail->next = nullptr;
		delete t;
		return;
	}
	t->next->prev = t->prev;
	t->prev->next = t->next;
	delete t;
	return;
}

void sort_time_list::tick() {
	if(!head) return;
	//获取当前时间
	time_t cur = time(nullptr);
	/*
	 * 从头节点 开始 依次处理 判断有没有定时任务 是否执行
	 */
	while(head){
		if(cur < head->expire){
			break;
		}
		head->cb_func(head->user_data);
		timer *tmp = head;
		head = head->next;
		delete tmp;
		if(head) head->prev = nullptr;
	}
}

void sort_time_list::addTimer(timer *t, timer *lst) {
	//lst第一个节点 一定比t小
	timer *prev = lst;
	timer *tmp = lst->next;
	while(tmp){
		if(tmp->expire > t->expire){
			prev->next = t;
			t->prev = prev;
			t->next = tmp;
			tmp->prev = t;
			break;
		}
		prev = tmp;
		tmp = tmp->next;
	}
	//链表中 没有比t大的 插入在最后
	if(!tmp){
		prev->next = t;
		t->prev = prev;
		t->next = nullptr;
		tail = t;
	}
}


