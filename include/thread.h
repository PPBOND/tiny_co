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
    void* arg = NULL;
    Fun fun = NULL;
    unsigned int co_id;
    ucontext_t   context; 
    bool is_end = false; 
    char stack[Default_size];
    Status status = Status::INIT;
    Event ev;
} co_struct;

//调用栈结构，保存被调方与调用方的链接关系
typedef struct co_env
{
     co_struct* call_stack[128];
     int index;
     Epoll_event ev_manger;
}co_env;

typedef struct time_co
{
    co_struct * co;
    struct timeval tv;
}time_co;



//调用栈关系
extern co_env  env;

//用于保存所有协程
extern std::deque<co_struct*> co_deques;

//准备就绪的协程跟正在运行的协程
extern std::deque<co_struct*> work_deques;

//主进程上下文,主要用来保存切换的上下文
extern co_struct co_main;

//协程休眠存放的链表
extern std::list<time_co* > sleep_list;

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




