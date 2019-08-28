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

 
enum class Status {INIT=1,READY,RUNNING,SLEEPING,EXIT};
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
} co_struct;

//调用栈结构，保存被调方与调用方的链接关系
typedef struct co_env
{
     co_struct* call_stack[128];
     int index;
}co_env;

typedef struct time_co
{
    co_struct * co;
    struct timeval tv;
}time_co;


void co_yield();
void schedule();
void co_resume(co_struct* co);
void co_sleep(int sleep_time);
void co_releae(co_struct* co);
void ready_co_to_queue();
co_struct* get_current();
int  co_create(co_struct* &co, Fun func, void *arg);




