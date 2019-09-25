#pragma once
#include "event.h"
#include "comm_tools.h"
#include <iostream>
#include <ucontext.h>
#include <string.h>
#include <queue>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <list>

#include <errno.h>




 
enum class Status {INIT=1, READY, RUNNING, SLEEPING, WAITING, EXIT};
typedef void (*Fun)(void* arg);
#define Default_size 8096



//协程结构体
typedef struct co_struct
{
    Event ev;
    void* arg = NULL;
    Fun fun = NULL;
    unsigned int co_id;
    ucontext_t   context; 
    bool is_end = false; 
    char stack[Default_size];
    Status status = Status::INIT;
    struct timeval tv;
    int get_time_with_usec() const
    {
        return tv.tv_sec*1000000 + tv.tv_usec;
    }
    
} co_struct;

//调用栈结构，保存被调方与调用方的链接关系
typedef struct co_env
{
     co_struct* call_stack[128];
     int index;
     Epoll_event ev_manger;
}co_env;


struct cmp_time
{
    bool operator()(co_struct* &lhs, co_struct* &rhs)
    {
        return lhs->get_time_with_usec() > rhs->get_time_with_usec();
    }

};

//调用栈关系
extern co_env  env;

//用于保存所有协程
extern std::deque<co_struct*> co_deques;

//准备就绪的协程跟正在运行的协程
extern std::deque<co_struct*> work_deques;

//主进程上下文,主要用来保存切换的上下文
extern co_struct co_main;

//协程休眠存放的
extern std::priority_queue<co_struct*, std::vector<co_struct*>, cmp_time> time_queue;

//协程等待时需要用到,唤醒则在epoll_wait后.
extern std::list<co_struct *> wait_list;





void co_init();
void co_yield();
void schedule();
void co_resume(co_struct* co);
void co_sleep(int sleep_time);
void co_releae(co_struct* co);
void ready_co_to_queue();
co_struct* get_current();

void ev_register_to_manager(co_struct * co);
int  co_create(co_struct* &co, Fun func, void *arg);
ssize_t co_recv(int fd, void *buf, size_t len);
int co_accept(int fd ,struct sockaddr* addr, socklen_t *len);




