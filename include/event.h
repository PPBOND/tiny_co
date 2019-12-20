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

     Event(int _fd, int _events,int _ops):event_timer(on_timeout,get_current(), 5,ONCE_EXEC),
     epoll_ev.data.fd(fd),epoll_ev.data.ptr(get_current()),
     events(_events),ops(_ops)
     {

     }
    //将fd,事件类型注册到event中
    int set_event_status(int sockfd , int events, int ops);
    int fd() { return epoll_ev.data.fd; }
    int res_event() { return res_status;}
    int get_event_status() { return  epoll_ev.events;}
    int update_time(int timeout)
    {
         event_timer.update_time(timeout);
        addtimer(event_timer);
    }
    
    int ops;
    int res_status;
    Time_Event event_timer;
    struct epoll_event epoll_ev;
    


};


class Epoll_event
{
public:

    int  create(int maxev = 20, int time_out = 10000);
    int  updateEvent(Event * ev,int time_out);
    int  wait_event();
    void wake_event();
    int  get_min_time();

    int  maxevent;
    int  timeout;  //取决于协程最小睡眠时间，否则默认为5秒
    int  epoll_fd;
    int  active_num;
    struct epoll_event active_ev[100];
    
};

