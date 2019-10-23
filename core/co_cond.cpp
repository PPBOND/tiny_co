#include "co_cond.h"

void co_cond_t::cond_wait()
{
    co_struct * current = get_current();
    current->status = Status::WAITING;
    wait_queue.push(current);
    ++size;
    co_yield();
}

void co_cond_t::cond_wake_once()
{
    while(size == 0)
        co_yield();
    co_struct * need_wake_co = wait_queue.front();
    need_wake_co->status = Status::READY;
    need_wake_co->is_timeout = false;
    --size;
    wait_queue.pop();
}


void co_cond_t::cond_wake_all()
{
    while(!wait_queue.empty())
    {
        co_struct* need_wake_co = wait_queue.front();
        need_wake_co->is_timeout = false;
        need_wake_co->status = Status::READY;
        wait_queue.pop();

    }
    size = 0;
}

int  co_cond_t::cond_time_wait(int time)
{
    co_struct * current_co = get_current();
    gettimeofday(&current_co->tv,NULL);
    current_co->tv.tv_sec += time;
    current_co->status = Status::SLEEPING;
    time_queue.push(current_co);
    wait_queue.push(current_co);
    ++size;
    co_yield();

    if(current_co->is_timeout)
    {   --size;
        remove_elem_from_queue(wait_queue, current_co);
        return -1;
    }

        remove_elem_from_queue(time_queue, current_co);
    return 0;
}

