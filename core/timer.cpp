#include "timer.h"
#include "comm_tools.h"
#include "coroutine.h"


int Timer_Manager::addTimeEvent(Time_event * time_event)
{
    time_event.update_time_cycle();
    m_min_heap.push(timer_elem);
    return 0;
}


int Timer_Manager::delTimeEvent(Time_event *time_event)
{
    if(time_event == NULL) 
        return -1;

    m_min_heap.remove(time_event);
    delete time_event;
    time_event = NULL;
    return 0;
}

void Timer_Manager::checkExpire()
{

    LOG_DEBUG("min_heap.size=%d", m_min_heap.size());
    while (m_min_heap.size() > 0)
    {
        
        Time_event *top_timer = m_min_heap.top();       
        if (top_timer->ms_time() < time_now()){

            LOG_DEBUG("timeout need exec\n");
            top_timer->is_timout = true;
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
            this->delTimeEvent(top_timer);
    }
    return;
}

struct timeval Timer_Manager::get_mix_time()
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
