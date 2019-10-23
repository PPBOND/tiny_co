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
    --size;
    wait_queue.pop();
}


void co_cond_t::cond_wake_all()
{
    while(!wait_queue.empty())
    {
        co_struct* need_wake_co = wait_queue.front();
        need_wake_co->status = Status::READY;
        wait_queue.pop();
    }
    size = 0;
}

