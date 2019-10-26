#include "co_cond.h"

void co_cond_t::cond_wait()
{
    co_struct *current = get_current();
    current->status    = Status::WAITING;
    wait_queue.push_front(current);
    co_yield();
}

void co_cond_t::cond_wake_once()
{
    while (wait_queue.size() == 0)
        co_yield();
    co_struct *need_wake_co = wait_queue.front();
    need_wake_co->status    = Status::READY;
    wait_queue.pop_front();
}

void co_cond_t::cond_wake_all()
{
    while (!wait_queue.empty())
    {
        co_struct *need_wake_co = wait_queue.front();
        need_wake_co->status    = Status::READY;
        wait_queue.pop_front();
    }
}

void wake_cond_co(void *co)
{
    co_struct *current_co = (co_struct *)co;
    current_co->status    = Status::READY;
    co_resume(current_co);
}

int co_cond_t::cond_time_wait(int time)
{
    TimerElem *cond_time;
    co_struct *current_co = get_current();
    current_co->status    = Status::SLEEPING;
    wait_queue.push_front(current_co);
    cond_time = co_centor.time_manager.AddTimer(wake_sleep_co, current_co, time, 0);
    co_yield();

    if (cond_time->isexec)
    {
        remove_elem_from_queue(wait_queue, current_co);
        return -1;
    }

    co_centor.time_manager.DelTimer(cond_time);
    return 0;
}
