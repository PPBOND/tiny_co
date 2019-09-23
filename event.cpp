#include "event.h"
#include "thread.h"


 int Event::init_event(int sock_fd , int events, int ops)
 {
    epoll_ev.events  = events; 
    epoll_ev.data.fd = sock_fd;
    this->ops  = ops;
 }




int Epoll_event::updateEvent( Event * ev)
{
    LOG_DEBUG("ev->epoll_ev.data.fd =%d", ev->epoll_ev.data.fd);
    epoll_ctl(epoll_fd, ev->ops , ev->epoll_ev.data.fd, &ev->epoll_ev);
    return 0;

}


int Epoll_event::create(int maxev, int time_out)
{  
   maxevent = maxev;
   timeout  = time_out;
   epoll_fd = epoll_create(1);
   
   return 0;
}


int Epoll_event::wait_event()
{
    printf("epoll wait\n");
    active_num = epoll_wait(epoll_fd, active_ev, maxevent, 10000000);
    return_if(active_num < 0, "epoll_wait error");
    return active_num;
}