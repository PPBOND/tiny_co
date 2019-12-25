#include "timer.h"
#include "comm_tools.h"
#include "coroutine.h"


int Timer_Manager::add_timer(Time_Event * time_event)
{
    
    if(time_event->in_heap == true)
        remove_timer(time_event);
    else 
        time_event->in_heap = true;
    
    m_min_heap.push(time_event);
    
    return 0;
}


int Timer_Manager::del_timer(Time_Event *time_event)
{
    if(time_event == NULL)
        return -1;

    remove_timer(time_event);
    delete time_event;
    
    return 0;
}

int  Timer_Manager::remove_timer(Time_Event * time_event)
{
    return m_min_heap.remove(time_event);
}


void Timer_Manager::check_expired()
{

    ////LOG_DEBUG("min_heap.size=%d", m_min_heap.size());
    while (m_min_heap.size() > 0)
    {
        
        Time_Event *top_timer = m_min_heap.top();       
        if (top_timer->ms_time() < time_now()){

            //LOG_DEBUG("timeout need exec\n");
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

long Timer_Manager::get_mix_time()
{
    return  m_min_heap.top()->ms_time();
     
}

int Timer_Manager::size()
{
    return m_min_heap.size();
}

bool Timer_Manager::empty()
{
    return m_min_heap.size() == 0;
}
