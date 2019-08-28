#pragma once

#include<iostream>
#include<ucontext.h>
#include <string.h>
#include<queue>
#include<sys/time.h>
#include <stdio.h>
#include <string.h>
#define _DEBUG_
#ifdef  _DEBUG_
#define LOG_DEBUG(fmt,...) \
printf("[DEBUG co_id=%d:%s:%d:%s] " fmt"\n",get_current()->co_id,__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#endif

#ifndef _DEBUG_
#define LOG_DEBUG(fmt,...) ;
#endif

 
enum class Status {INIT = 1,READY,RUNNING,SLEEPING,EXIT};
typedef void (*Fun)(void* arg);
#define Default_size 8096



//协程结构体
typedef struct co_struct
{
    void* arg = NULL;
    Fun fun   = NULL;
    unsigned int co_id;
    ucontext_t context; 
    bool is_end = false; 
    char stack[Default_size];
    Status status = Status::INIT;
} co_struct;

//调用栈结构，保存被调方与调用方的链接关系
typedef struct co_env
{
     co_struct* call_stack[128];
     int index;
}co_env;

//管理协程休眠结构ß
typedef struct time_co
{
    co_struct * co;
    struct timeval tv;
}time_co;

//协程主动yield,将控制权移交给call方
void co_yield();

//协程调度器，生产者消费者模式，sleep进程唤醒，协程队列遍历就绪协程，然后进行调度
void schedule();
//将自己挂起，运行co指定协程
void co_resume(co_struct* co);
//将协程睡眠，并将控制权转移到call方，而调度器将调度其他协程。
void co_sleep(int sleep_time);
//协程运行结束后，将被释放内存
void co_releae(co_struct* co);
//将就绪协程，移动到就绪队列
void ready_co_to_queue();
//获取当前协程。
co_struct* get_current();
//主协程构建上下文，初始化
void co_main_init();
//创建协程
int  co_create(co_struct* &co, Fun func, void *arg);




