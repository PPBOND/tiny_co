#include "co_cond.h"

void co_cond_t::cond_wait()
{
    CoRoutine_t *current_co = get_current();
    current_co->status      = Status::waiting;
    cond_wait_queue.push_back(current_co);
    co_yield();
}

void co_cond_t::cond_wake_once()
{
    while (cond_wait_queue.size() == 0)
        co_yield();

    CoRoutine_t* wake_co  = cond_wait_queue.front();
    wake_co->status       = Status::ready;

    cond_wait_queue.pop_front();
    sche_centor.ready_manager.push_back(wake_co);
    
}

void co_cond_t::cond_wake_all()
{
    while (!cond_wait_queue.empty())
    {
        CoRoutine_t* wake_co = cond_wait_queue.front();
        wake_co->status    = Status::ready;
        cond_wait_queue.pop_front();
        sche_centor.ready_manager.push_back(wake_co);
    }
}


int co_cond_t::cond_time_wait(int time)
{
    TimerElem *cond_time;
    CoRoutine_t *current_co = get_current();
    current_co->status    = Status::sleeping;
    cond_wait_queue.push_back(current_co);
    cond_time = sche_centor.time_manager.addtimer(wake_sleep_co, current_co, time, ONCE_EXEC);
    co_yield();

    if (cond_time->isexec)
    {
        remove_elem_from_queue(cond_wait_queue, current_co);
        return -1;
    }

    sche_centor.time_manager.deltimer(cond_time);
    return 0;
}
