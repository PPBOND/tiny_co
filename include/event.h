#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "comm_tools.h"
#include "socket.h"
class Event
{
public:
    int init_event(int sockfd , int events, int ops);
    int get_fd() { return epoll_ev.data.fd; }
    int get_event() { return  epoll_ev.events;}

    int ops;
    struct epoll_event epoll_ev;
    int fd;
};


class Epoll_event
{
public:

    int create(int maxev = 20, int time_out = 10000);
    int updateEvent(Event * ev);
    int wait_event();


    int maxevent;
    int timeout;
    int epoll_fd;
    int active_num;
    struct epoll_event active_ev[100];
    
};

