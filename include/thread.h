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

#define Default_size 8096
enum class Status {INIT=1, READY, RUNNING, SLEEPING, WAITING, EXIT};
using  Fun = void(*)(void* arg);


//协程结构体
struct co_struct
{
    Event ev;
    struct timeval tv;
    unsigned int   co_id;
    ucontext_t     context;
    char stack[Default_size];

    Fun fun       = NULL;
    void* arg     = NULL;
    bool is_end   = false; 
    Status status = Status::INIT;
    
    int get_time_with_usec() const
    {
        return tv.tv_sec*1000000 + tv.tv_usec;
    }
    
};

//调用栈结构，保存被调方与调用方的链接关系
struct co_dispatch_centor
{
    int index;
    Epoll_event ev_manger;
    co_struct*  call_stack[128];
};


struct cmp_time
{
    bool operator()(co_struct* &lhs, co_struct* &rhs)
    {
        return lhs->get_time_with_usec() > rhs->get_time_with_usec();
    }

};



/*
以下为全局链表跟队列声明，event类需要用到,定义在thread.cpp中
*/
extern std::list<co_struct *> wait_list;
extern std::priority_queue<co_struct*, std::vector<co_struct*>, cmp_time> time_queue;

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
int  co_read(int fd, void* buf, size_t len);
int  co_write(int fd, char* buf, size_t len);
int  co_accept(int fd ,struct sockaddr* addr, socklen_t *len);




