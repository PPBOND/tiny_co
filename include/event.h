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
#include "socket.h"



class Event
{
public:
    int alter_status(int sockfd , int events, int ops);
    int get_fd() { return epoll_ev.data.fd; }
    int get_event() { return  epoll_ev.events;}
    
    int ops;
    struct epoll_event epoll_ev;
};


class Epoll_event
{
public:

    int  create(int maxev = 20, int time_out = 10000);
    int  updateEvent(Event * ev);
    int  wait_event();
    void wake_event();
    int  get_min_time();

    int  maxevent;
    int  timeout;  //取决于协程最小睡眠时间，否则默认为5秒
    int  epoll_fd;
    int  active_num;
    struct epoll_event active_ev[100];
    
};

