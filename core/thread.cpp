#include "thread.h"



using namespace std;



//协程调度中心
co_dispatch_centor  co_centor;

//Todo UUID
static int generator_uuid =0;
static int get_uuid() { return ++generator_uuid; }

//用于保存所有协程
std::deque<co_struct*> total_co_deques;

//准备就绪的协程跟正在运行的协程
std::deque<co_struct*> work_deques;

//主进程上下文,主要用来保存切换的上下文
co_struct co_main;

//协程休眠存放的链表
std::priority_queue<co_struct*, std::vector<co_struct*>, cmp_time> time_queue;

//协程等待时需要用到,唤醒则在epoll_wait后.
std::list<co_struct *> wait_list;



void wake_sleeping_co()
{
    struct timeval  tv;
    gettimeofday(&tv,NULL);   

    while(!time_queue.empty())
    {
        co_struct * top_co = time_queue.top();
        int diff_time = top_co->get_time_with_usec() - (tv.tv_sec*1000000 + tv.tv_usec);
        LOG_DEBUG("diff_time =%d",diff_time);
        if(diff_time < 0)
        {
            time_queue.pop();
            top_co->status = Status::READY;
            work_deques.push_back(top_co);
            
        }
        else
            return ;
    }

}



co_struct* get_current()
{
    return co_centor.call_stack[co_centor.index-1];
}


void co_func(co_struct* co)
{
    if(co->fun)
    {
        Fun process = co->fun;
        process(co->arg);
    }

    co_struct * current = co_centor.call_stack[co_centor.index-1];
    current->is_end     = true;
    LOG_DEBUG("over");
    co_yield();
}

void co_init()
{
    if(co_centor.call_stack[0] == NULL)
    {
        getcontext(&co_main.context);
        co_main.co_id  = get_uuid();
        co_main.status = Status::RUNNING;
        co_centor.call_stack[co_centor.index++] = &co_main;
        co_centor.ev_manger.create(20,1000);
    }
}



int co_create(co_struct* &co, Fun func, void *arg)
{
    
    co = new co_struct;
    getcontext(&co->context);
    co->arg    = arg;
    co->fun    = func;
    co->co_id  = get_uuid();
    co->status = Status::READY;
    co->context.uc_stack.ss_sp    = co->stack;
    co->context.uc_stack.ss_size  = Default_size;
    co->context.uc_stack.ss_flags = 0;
    co->context.uc_link = NULL;

    makecontext(&co->context, (void (*)())co_func, 1, co);  
    total_co_deques.push_back(co);

    return 0;
}




int co_timer(co_struct* &co, Fun func, void *arg,unsigned int time)
{
    co_create(co, func,arg);
    gettimeofday(&co->tv,NULL);
    co->tv.tv_sec += time;
    co->status     = Status::SLEEPING;
    time_queue.push(co);
    return 0;

}
void ready_co_to_queue()
{
    if(!work_deques.empty())
        return;
    
    for(auto& co: total_co_deques)
    {
        if(co->status == Status::READY)
        {
            LOG_DEBUG("co->id=%d",co->co_id);
            work_deques.push_back(co);
            LOG_DEBUG("work.size()=%zu", work_deques.size());
        }
    }
}

void schedule()
{
    
    while(1)
    {
        wake_sleeping_co();
        ready_co_to_queue();
        if(!work_deques.empty())
            goto ready_co_run;

        if(work_deques.empty() && time_queue.empty() && wait_list.empty())
        {
            LOG_DEBUG("work_deues.empty, schedule finish");
            return;
        }

        co_centor.ev_manger.wait_event();
        co_centor.ev_manger.wake_event();        
        ready_co_to_queue();
        if(work_deques.empty())
            continue;

ready_co_run:
        co_struct* next_co = work_deques.front();
        work_deques.pop_front(); 

        LOG_DEBUG("next_co= %d", next_co->co_id);
        co_resume(next_co);

        if(next_co->status == Status::EXIT)
            co_releae(next_co);

        LOG_DEBUG("schedule end ");

    }

}



void  co_yield()
{
    co_struct * current  = co_centor.call_stack[co_centor.index -1];
    co_struct * prev     = co_centor.call_stack[co_centor.index -2];
    LOG_DEBUG("will runing=%d", prev->co_id);
    co_centor.index--;

    if(current->is_end == true)
        current->status = Status::EXIT;
    else if(current->status == Status::RUNNING) 
        current->status = Status::READY;

    prev->status = Status::RUNNING;
    swapcontext(&current->context, &prev->context);
    current->status = Status::RUNNING;

}



void co_resume(co_struct* ready_co)
{
    if(ready_co->status != Status::READY)
    {
        LOG_DEBUG("co->status != READY,=%d",ready_co->status);
        return;
    }

    co_centor.call_stack[co_centor.index++] = ready_co;
    co_struct * prev  = co_centor.call_stack[co_centor.index -2];
    ready_co->status  = Status::RUNNING;    
    prev->status      = Status::READY;
    swapcontext(&prev->context, &ready_co->context);
    prev->status      = Status::RUNNING;
   
}



void co_releae(co_struct* release_co)
{
    if(release_co->status != Status::EXIT)
        return;

    for(auto iter = total_co_deques.begin(); iter != total_co_deques.end();)
    {
        if(*iter == release_co)
        {
            LOG_DEBUG("co_id=%d release", release_co->co_id);
            iter = total_co_deques.erase(iter);
            delete release_co;
        }
        else
            iter++;
    }

    LOG_DEBUG("total_co_deques.size=%ld", total_co_deques.size());
}



void ev_register_to_manager(int fd, int event,int ops)
{
    LOG_DEBUG("ev_register_to_manager fd =%d, event=%d, ops=%d",fd, event, ops );
    co_struct* current_co = get_current();
    current_co->ev.alter_status(fd, event, ops);
    co_centor.ev_manger.updateEvent(&current_co->ev);
    current_co->status = Status::WAITING;
    wait_list.push_back(current_co);
    co_yield();
}

