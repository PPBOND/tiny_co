#include "timer.h"
#include "comm_tools.h"
#include "coroutine.h"

TimerElem *Timer_Manager::addtimer(FuncPtrOnTimeout expired_func, void *data,
                                   uint64_t expired_ms, int flag)
{
    TimerElem *timer_elem = (TimerElem *)malloc(sizeof(TimerElem));
    timer_elem->expired_func_ = expired_func;
    timer_elem->data_ = data;
    timer_elem->isexec = false;
    timer_elem->cycle_flag = flag;
    timer_elem->expired_ms_ = expired_ms;
    gettimeofday(&timer_elem->tv, NULL);
    timer_elem->tv.tv_sec += expired_ms;
    LOG_DEBUG("AddTimer= %d %d ", timer_elem->tv.tv_sec, timer_elem->tv.tv_usec);
    m_min_heap.push(timer_elem);
    return timer_elem;
}

int Timer_Manager::deltimer(TimerElem *timer_elem)
{
    if(timer_elem == NULL) 
        return -1;

    m_min_heap.remove(timer_elem);
    delete timer_elem;
    timer_elem = NULL;
    return 0;
}

void Timer_Manager::checkexpire()
{

    LOG_DEBUG("min_heap.size=%d", m_min_heap.size());
    while (m_min_heap.size() > 0)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        struct timeval min_tv = get_mix_time();
        TimerElem *top_elem = m_min_heap.top();
        long long timeout_flag = (min_tv.tv_sec - tv.tv_sec) * 1000 + (min_tv.tv_usec - tv.tv_usec) / 1000;
       
        if (timeout_flag < 0)
        {
            LOG_DEBUG("timeout need exec\n");
            top_elem->isexec = true;
            top_elem->expired_func_(top_elem->data_);
        }
        else
            return;

        if (top_elem->cycle_flag == 1)
        {
            m_min_heap.pop();
            top_elem->tv.tv_usec = tv.tv_usec;
            top_elem->tv.tv_sec = tv.tv_sec + top_elem->expired_ms_;
            m_min_heap.push(top_elem);
        }
        else
            this->deltimer(top_elem);
    }
    return;
}

struct timeval Timer_Manager::get_mix_time()
{
    TimerElem *top_elem = m_min_heap.top();
    return top_elem->tv;
}

int Timer_Manager::size()
{
    return m_min_heap.size();
}

bool Timer_Manager::empty()
{
    return m_min_heap.size() == 0;
}
