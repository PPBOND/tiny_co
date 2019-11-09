#include "event.h"
#include "coroutine.h"
#include <sys/socket.h>
#include "Logger.h"

int Event::alter_status(int sock_fd , int events, int ops)
{
    epoll_ev.events  = events; 
    epoll_ev.data.fd = sock_fd;
    this->ops  = ops;
    return 0;
}


int Epoll_event::updateEvent( Event * ev)
{
   // LOG_DEBUG("ev->epoll_ev.data.fd =%d", ev->epoll_ev.data.fd);
    int ret= epoll_ctl(epoll_fd, ev->ops , ev->epoll_ev.data.fd, &ev->epoll_ev);
    //LOG_DEBUG("-----------------------------------------ret=%d",ret);
    return ret;
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
    int wait_time = get_min_time();
    LOG_DEBUG("epoll_wait`s timeout=%d\n", wait_time);
    active_num = epoll_wait(epoll_fd, active_ev, maxevent, wait_time) ;
    return_if(active_num < 0, "epoll_wait error");
    LOG_DEBUG("epoll wait, active_num = %d\n", active_num);
    return active_num;
}

void Epoll_event::wake_event()
{
    for(int i = 0; i< this->active_num; ++i)
    {
       
        int active_fd = active_ev[i].data.fd;
        LOG_DEBUG("active_fd = %d \n", active_fd);

        for(auto list_node =  sche_centor.wait_manager.begin(); list_node != sche_centor.wait_manager.end();)
        {
            int event_fd = (*list_node)->ev.get_fd();
            LOG_DEBUG("event_fd =%d \n", event_fd);

            if(event_fd == active_fd)
            {   
                sche_centor.ready_manager.push_back(*list_node);
                (*list_node)->status = Status::ready;
                (*list_node)->ev.alter_status(active_fd,active_ev[i].events,EPOLL_CTL_DEL);
                updateEvent(&(*list_node)->ev);
                list_node = sche_centor.wait_manager.erase(list_node);
                
                break;
            }
            ++ list_node;
        }
    }

}



int Epoll_event::get_min_time()
{
    struct timeval  tv;
    gettimeofday(&tv,NULL);   

    if(sche_centor.time_manager.empty())
        return 5000;

    struct timeval  mix_time = sche_centor.time_manager.get_mix_time();
    int time_diff = (mix_time.tv_sec - tv.tv_sec)*1000 + (mix_time.tv_usec - tv.tv_usec)/1000;
    LOG_DEBUG("time_diff =%d", time_diff);
    return time_diff > 0 ? time_diff : 0; 

}