#pragma once
#include <deque>
#include "thread.h"
class co_cond_t
{
public:
    co_cond_t():size(0){}
    ~co_cond_t(){}
    void cond_wait();
    void cond_wake_once();
    void cond_wake_all();

    std::queue<co_struct*> wait_queue;
    int size;

};