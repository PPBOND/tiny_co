#include "event.h"
#include "coroutine.h"
#include <sys/socket.h>
#include "Logger.h"

int event_t::set_event(int sock_fd, int events)
{
    if(epoll_ev.events == events)
        ev_changed = false;
    else 
        ev_changed = true;

    if(in_loop)
        ops = EPOLL_CTL_MOD;
    else
        ops = EPOLL_CTL_ADD;

    this->fd = sock_fd;
    epoll_ev.events  = events; 
    
    
    return 0;
}


int event_manager_t::update_event(event_t* ev,int timeout)
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

int event_manager_t::remove_event(event_t* ev)
{
    if(ev->in_loop){
        ev->remove_timer();
        ev->in_loop = false;
        return epoll_ctl(epoll_fd, EPOLL_CTL_DEL , ev->fd(), &ev->epoll_ev);
    }
    return 0;
}


int event_manager_t::create(int maxev, int time_out)
{  
   maxevent = maxev;
   timeout  = time_out;
   epoll_fd = epoll_create(1);
   return 0;
}


int event_manager_t::wait_event()
{
    int wait_time = get_min_time();
    active_num = epoll_wait(epoll_fd, active_ev, maxevent, wait_time) ;
    return active_num;
}

void event_manager_t::wake_event()
{
    for(int i = 0; i< this->active_num; ++i)
    {
        event_t* ev = (event_t*)active_ev[i].data.ptr;
        ev->ret_status = active_ev[i].events;        
        coroutine_t* active_co = ev->co;
        active_co->status = status::ready;
        schedule_centor::remove_wait_list(active_co);
        schedule_centor::add_ready_queue(active_co);
}

}



int event_manager_t::get_min_time()
{

    struct timeval  tv;
    gettimeofday(&tv,NULL);   

    if(schedule_centor::timer_manager.empty())
        return 5000;

    int time_diff =schedule_centor::timer_manager.get_mix_time() - time_now();
    return time_diff > 0 ? time_diff : 0; 
	return 0;
}
