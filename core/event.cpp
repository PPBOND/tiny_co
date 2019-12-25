#include "event.h"
#include "coroutine.h"
#include <sys/socket.h>
#include "Logger.h"

int Event::set_event(int sock_fd, int events)
{
    if(epoll_ev.events == events)
        ev_changed = false;
    else 
        ev_changed = true;

    if(in_loop)
        ops = EPOLL_CTL_MOD;
    else
        ops = EPOLL_CTL_ADD;

    epoll_ev.events  = events; 
    epoll_ev.data.fd = sock_fd;
    

   
    
    return 0;
}


int Epoll_event::update_event(Event * ev,int timeout)
{
     //负数表示不需要定时加入
    if(timeout >= 0)
        ev->update_time(timeout);

    if(ev->ev_changed){   
        ev->in_loop = true;
        ev->ev_changed = false;
        return epoll_ctl(epoll_fd, ev->ops, ev->fd(), &ev->epoll_ev);
    }
    
    return 0;
}

int Epoll_event::remove_event(Event* ev)
{
    if(ev->in_loop){
        ev->remove_timer();
        ev->in_loop = false;
        return epoll_ctl(epoll_fd, EPOLL_CTL_DEL , ev->fd(), &ev->epoll_ev);
    }
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

        CoRoutine_t* ready_co = (CoRoutine_t*)active_ev[i].data.ptr;
        ready_co->status = Status::ready;
        sche_centor.wait_manager.remove(ready_co);
        sche_centor.ready_manager.push_back(ready_co);
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