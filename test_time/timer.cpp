#include "timer.h"

static heap<TimerElem*,cmp_time>  min_heap;



TimerElem * CTimerManager::AddTimer(FuncPtrOnTimeout expired_func, void *data,
                      uint64_t expired_ms, int flag)
{
    TimerElem *timer_elem = (TimerElem *)malloc(sizeof(TimerElem));
    timer_elem->expired_func_ = expired_func;
    timer_elem->data_ = data;
    timer_elem->cycle_flag = flag;
    timer_elem->expired_ms_ = expired_ms;
    gettimeofday(&timer_elem->tv,NULL);
    timer_elem->tv.tv_sec += expired_ms;
    min_heap.push(timer_elem);
    return timer_elem;
}     


int CTimerManager::DelTimer(TimerElem *timer_elem)
{
    if (timer_elem == NULL) {
      return -1;
    }
    min_heap.remove(timer_elem);
    delete timer_elem;
    return 0;

}

bool CTimerManager::CheckExpire()
{
  
    while(min_heap.size()> 0)
    {
        struct timeval  tv;
        gettimeofday(&tv,NULL); 
     
        TimerElem * top_elem = min_heap.top();
        if(top_elem->tv.tv_sec < tv.tv_sec )
        {
            top_elem->expired_func_(top_elem->data_);
            min_heap.pop();
        }
        else 
            break;

        if(top_elem->cycle_flag == 1)
        {
            top_elem->tv.tv_sec = tv.tv_sec + top_elem->expired_ms_;
            min_heap.push(top_elem);
        }
    }
}


 struct timeval CTimerManager::get_mix_time()
 {
    TimerElem * top_elem = min_heap.top();
    return top_elem->tv;

 }

int CTimerManager::get_elem_size()
{
    return min_heap.size();
}