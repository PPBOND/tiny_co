#pragma once
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include "comm_tools.h"
#include "schedule.h"



class event_t
{
public:

    //超时回调唤醒协程,同时设置事件为超时状态
    static void on_timeout(void *data){
        event_t* ev = (event_t*)data;
        ev->ret_status = EVENT_TIMEOUT;
        coroutine_t* time_co = ev->co;
        time_co->co_status = status::ready;
    }

    event_t():event_timer(on_timeout, this, SOCK_TIMEOUT ,ONCE_EXEC),in_loop(false),
    ev_changed(true),fd(-1)
    {
        epoll_ev.data.ptr = this;
        epoll_ev.events = -100;
        ops = -1;
        ret_status = EVENT_NONE;
    }

    event_t(int _fd, int _events):
        event_timer(on_timeout,this, SOCK_TIMEOUT ,ONCE_EXEC),
        fd(_fd),in_loop(false),ev_changed(true)
     {
        epoll_ev.events =_events;
        epoll_ev.data.ptr = this;
        ops = -1;
        ret_status = EVENT_NONE;
     }

    //将fd,事件类型注册到event中
    int set_event(int sockfd , int events);
    
    int ret_event() { 
        return ret_status;
    }
    
    int event_status() {
        return  epoll_ev.events;
    }
    
    int update_time(int timeout)
    {
        event_timer.update_time(timeout);    
        schedule_centor::add_timer(&event_timer);
    }

   
    int remove_timer(){
        return schedule_centor::remove_timer(&event_timer);
    }
    
    int fd;
    int  ops;
    bool in_loop;
    bool ev_changed;
    int  ret_status;
    timer_event_t event_timer;
    struct epoll_event epoll_ev;
};


class event_manager_t
{
public:

    int  create(int maxev = 20, int time_out = 10000);
    int  update_event(event_t* ev,int time_out);
    int  remove_event(event_t* ev);
    int  wait_event();
    void wake_event();
    int  get_min_time();

    int  maxevent;
    int  timeout;  //取决于协程最小睡眠时间，否则默认为5秒
    int  epoll_fd;
    int  active_num;
    struct epoll_event active_ev[100];
    
};



