#pragma once
#include "event.h"
#include "comm_tools.h"
#include <iostream>
#include <ucontext.h>
#include <string.h>
#include <queue>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "timer.h"
#include <list>
#include <errno.h>
#include "Logger.h"


extern int get_uuid();
#define Default_size 8096
enum class Status { init = 1, ready, running, sleeping,waiting, exit};
extern int swapcontext(ucontext_t *, ucontext_t *) asm("swapcontext");
extern int getcontext(ucontext_t *) asm("getcontext");
using Func = void* (*)(void* arg);



//协程结构体
struct CoRoutine_t
{
    CoRoutine_t() = default;
    CoRoutine_t(Func func, void *arg):routine(func),co_arg(arg),is_joinable(false){
        getcontext(&this->u_context);
        this->status = Status::ready;
        this->routine_id = get_uuid();
        this->u_context.uc_stack.ss_sp    = this->stack;
        this->u_context.uc_stack.ss_size  = Default_size;
        this->u_context.uc_stack.ss_flags = 0;
        this->u_context.uc_link = NULL; 
    }
    
public:
    Event ev;
    unsigned int   routine_id;
    ucontext_t     u_context;
    char stack[Default_size];
    Func  routine     = NULL;
    void* co_arg      = NULL;
    void* exit_ret    = NULL;
    bool  is_end      = false; 
    bool  is_joinable = false;
    Status status     = Status::init;

};



using Wait_Manager  = std::list<CoRoutine_t*>;
using Ready_Manager = std::deque<CoRoutine_t*>;

//保存被调方与调用方的链接关系,管理event事件与超时事件.
class Schedule_Centor
{

public:
    Event* alloc_event_by_fd(int fd){
        Event* ev = new Event();
        event_map[fd] = ev;
        return ev;
    }
    void shedule_run();
    void get_event()
    {
        CoRoutine_t * current_co = get_current();
        if(event_map[current_co].empty()){
            event_map[current_co()] = new Event();
        }
        return event_map[current_co];
    }
public:
    int chain_index;
    int generator_uuid =0;
    CoRoutine_t*   call_stack[128];
    Epoll_event    ev_manger;
    Timer_Manager  time_manager;
    Wait_Manager   wait_manager;
    Ready_Manager  ready_manager;
    CoRoutine_t    main_co;
    std::map<int ,Event*> event_map; 
};




extern Schedule_Centor  sche_centor;




void co_init();

void co_yield();
void event_loop_run();
void co_resume(CoRoutine_t* co);
void co_releae(CoRoutine_t* co);
void wake_sleep_co (void *co);
CoRoutine_t* get_current();
void ev_register_to_manager(int fd, int event,int ops);
int  co_create(CoRoutine_t* &co, Func func, void *arg, bool isjoin );
int  co_timer(CoRoutine_t* &co, Func func, void *arg,unsigned int time);
int  co_join(CoRoutine_t* &co, void** retval);


TimerElem * addtimer(FuncPtrOnTimeout expired_func, void *data,
                                   uint64_t expired_ms, int flag);
int deltimer(TimerElem *timer_elem);


template<class T, class Q>
void remove_elem_from_queue(T& queue, Q& current_co)
{
    for(auto iter = queue.begin(); iter != queue.end();)
    {
        if(*iter == current_co)
        {
            LOG_DEBUG("co_id=%d release", current_co->routine_id);
            iter = queue.erase(iter);        
        }    
        else
            iter++;
    }
    
}