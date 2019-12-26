#include "timer.h"
#include "comm_tools.h"


int timer_manager_t::add_timer(timer_event_t* time_ev)
{
    
    if(time_ev->in_heap == true)
        remove_timer(time_ev);
    else 
        time_ev->in_heap = true;
    
    m_min_heap.push(time_ev);
    return 0;
}


int timer_manager_t::del_timer(timer_event_t* time_ev)
{
    if(time_ev == NULL)
        return -1;
    
    remove_timer(time_ev);
    delete time_ev;
    
    return 0;
}

int  timer_manager_t::remove_timer(timer_event_t* time_ev)
{
    return m_min_heap.remove(time_ev);
}


void timer_manager_t::check_expired()
{

    while (m_min_heap.size() > 0)
    {
        
        timer_event_t *top_timer = m_min_heap.top();       
        if (top_timer->ms_time() < time_now()){
            top_timer->run();
        }
        else
            return;

        if (top_timer->cycle_flag == 1){
            m_min_heap.pop();
            top_timer->update_time_cycle();
            m_min_heap.push(top_timer);
        }
        else
            remove_timer(top_timer);
    }
    return;
}

long timer_manager_t::get_mix_time()
{
    return  m_min_heap.top()->ms_time();
     
}

int timer_manager_t::size()
{
    return m_min_heap.size();
}

bool timer_manager_t::empty()
{
    return m_min_heap.size() == 0;
}
