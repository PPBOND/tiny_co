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
#include "timer.h"
#include <list>
#include <errno.h>

#define Default_size 8096
enum class Status {INIT = 1, READY, RUNNING, SLEEPING, WAITING, EXIT};
using  Fun = void(*)(void* arg);


//协程结构体
struct co_struct
{
    Event ev;
    unsigned int   co_id;
    ucontext_t     context;
    char stack[Default_size];
    Fun fun         = NULL;
    void* arg       = NULL;
    bool is_end     = false; 
    Status status  = Status::INIT;
};

//调用栈结构，保存被调方与调用方的链接关系
struct co_dispatch_centor
{
    int index;
    Epoll_event ev_manger;
    CTimerManager time_manager;
    co_struct*  call_stack[128];
};





/*
以下为全局链表跟队列声明，event类需要用到,定义在thread.cpp中
*/
extern std::list<co_struct *> wait_list;
extern co_dispatch_centor  co_centor;
void co_init();
void co_yield();
void schedule();
void ready_co_to_queue();
void co_resume(co_struct* co);
void co_releae(co_struct* co);
void wake_sleep_co (void *co);
co_struct* get_current();
void ev_register_to_manager(int fd, int event,int ops);
int  co_create(co_struct* &co, Fun func, void *arg);
int  co_timer(co_struct* &co, Fun func, void *arg,unsigned int time);


TimerElem * addtimer(FuncPtrOnTimeout expired_func, void *data,
                                   uint64_t expired_ms, int flag);
int deltimer(TimerElem *timer_elem);