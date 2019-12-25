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



class Event
{
public:

    //超时回调唤醒协程,同时设置事件为超时状态
    static void on_timeout(void *data){
        CoRoutine_t* wake_co = (CoRoutine_t*)data;
        wake_co->ev.res_status = EVENT_TIMEOUT;
        wake_co->status = Status::ready;
        sche_centor.ready_manager.push_back(wake_co);
    }

    Event():event_timer(on_timeout,nullptr, SOCK_TIMEOUT ,ONCE_EXEC),in_loop(false),
    ev_changed(true)
    {
        //bind_context(get_current());
        epoll_ev.events = -100;
        epoll_ev.data.fd  = -1;
        epoll_ev.data.ptr = nullptr;
        ops = -1;
        res_status = EVENT_NONE;
    }

    Event(int _fd, int _events):
        event_timer(on_timeout,nullptr, SOCK_TIMEOUT ,ONCE_EXEC),
        in_loop(false),epoll_ev.data.fd(fd),ev_changed(true),
        epoll_ev.events(_events)
     {
        //bind_context(get_current());
        ops = -1;
        res_status = EVENT_NONE;
     }

    //将fd,事件类型注册到event中
    int set_event(int sockfd , int events);
    int fd() { 
        return epoll_ev.data.fd; 
    }
    
    int res_event() { 
        return res_status;
    }
    
    int event_status() {
        return  epoll_ev.events;
    }
    
    int update_time(int timeout)
    {
        event_timer.update_time(timeout);    
        add_timer(&event_timer);
    }

    //将协程与事件绑定
    void bind_context(){
        epoll_ev.data.ptr= this;
        event_timer.set_args(this);
    }

    int remove_timer(){
        remove_timer(&event_timer);
    }
    
    int ops;
    bool in_loop;
    bool ev_changed;
    int res_status;
    Time_Event event_timer;
    struct epoll_event epoll_ev;
    CoRoutine_t * co;

};


class Epoll_event
{
public:

    int  create(int maxev = 20, int time_out = 10000);
    int  update_event(Event* ev,int time_out);
    int  remove_event(Event* ev);
    int  wait_event();
    void wake_event();
    int  get_min_time();

    int  maxevent;
    int  timeout;  //取决于协程最小睡眠时间，否则默认为5秒
    int  epoll_fd;
    int  active_num;
    struct epoll_event active_ev[100];
    
};

