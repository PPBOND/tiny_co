#include "coroutine.h"
using namespace std;

//协程调度中心
co_dispatch_centor  co_centor;

//Todo UUID
static int generator_uuid =0;
static int get_uuid() { return ++generator_uuid; }


//准备就绪的协程跟正在运行的协程
std::deque<co_struct*> work_deques;

//主进程上下文,主要用来保存切换的上下文
co_struct co_main;

//协程等待时需要用到,唤醒则在epoll_wait后.
std::list<co_struct *> wait_list;



co_struct* get_current()
{
    return co_centor.call_stack[co_centor.index-1];
}


void co_func(co_struct* co)
{
    if(co->fun){
        Fun process = co->fun;
        co->exit_ret= process(co->arg);
    }
    co->is_end = true;
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



int  co_create(co_struct* &co, Fun func, void *arg, bool isjoin)
{
    
    co = new co_struct;
    getcontext(&co->context);
    co->arg    = arg;
    co->fun    = func;
    co->co_id  = get_uuid();
    co->is_joinable = isjoin;
    co->status = Status::READY;
    co->context.uc_stack.ss_sp    = co->stack;
    co->context.uc_stack.ss_size  = Default_size;
    co->context.uc_stack.ss_flags = 0;
    co->context.uc_link = NULL;

    makecontext(&co->context, (void (*)())co_func, 1, co);  
    work_deques.push_back(co);

    return 0;
}



void schedule()
{
    
    while(1)
    {
        /*sleep函数的实现:如果有睡眠协程,执行特定的事件:将修改睡眠协程状态为可执行状态,然后放入就绪队列
          如果是普通事件,将执行定时事件
        */
        co_centor.time_manager.CheckExpire();
        if(!work_deques.empty())
            goto ready_co_run;

        if(work_deques.empty() && co_centor.time_manager.empty() && wait_list.empty())
        {
            LOG_DEBUG("work_deues.empty, schedule finish");
            return;
        }

        /*
        等待IO事件发生,如果发生了则根据fd找到对应的协程,接着再将协程状态改变为可执行状态,然后放入就绪队列.
        等待时间则根据最小堆的top节点的时间进行等待,如果最小堆的节点为空,则默认为5秒.
        */
        co_centor.ev_manger.wait_event();
        co_centor.ev_manger.wake_event();        
        if(work_deques.empty())
            continue;

ready_co_run:
        /*
            执行就绪事件,首先从就绪队列弹出一个协程执行,协程执行完或者被yield()并没执行完,再恢复到
            主协程时做判断,如果没执行完主动yield则继续放入队列等待执行,如果执行完毕并不是joinable则释放资源(这里默认都是detach协程).
            如果是joinable则需要自己调用co_join的处理.这里不做处理.
        */
        while(!work_deques.empty())
        {
            co_struct* next_co = work_deques.front();
            work_deques.pop_front(); 
            LOG_DEBUG("next_co= %d\n", next_co->co_id);
            co_resume(next_co);

	        LOG_DEBUG("co_status=%d\n",next_co->status ); 
            if(next_co->status == Status::READY)
                work_deques.push_back(next_co);     
            else if(next_co->status == Status::EXIT && next_co->is_joinable != true)
                co_releae(next_co);
        }
       
       

    }
    
    LOG_DEBUG("schedule end ");

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


int co_join(co_struct* &co, void** retval)
{
    co_struct * current = get_current();
    if(co == current )  return 0;
    if(co->is_joinable == false)
        return -1;

    while(co->status != Status::EXIT)
        co_yield();
    
    if(retval != nullptr)  
        *retval = co->exit_ret;
    
    co_releae(co);

    return 0;
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

    printf("co_id=%d release\n", release_co->co_id);
    delete release_co;
      
}



void ev_register_to_manager(int fd, int event,int ops)
{
    LOG_DEBUG("ev_register_to_manager fd =%d, event=%d, ops=%d",fd, event, ops );
    co_struct* current_co = get_current();
    current_co->status    = Status::WAITING;
    current_co->ev.alter_status(fd, event, ops);
    co_centor.ev_manger.updateEvent(&current_co->ev);
    wait_list.push_back(current_co);
    co_yield();
}



void wake_sleep_co(void *co)
{
    co_struct* wake_co = (co_struct*) co;
    wake_co->status = Status::READY;
    work_deques.push_back(wake_co);
}


TimerElem * addtimer(FuncPtrOnTimeout expired_func, void *data,
                                   uint64_t expired_ms, int flag)
{
    return co_centor.time_manager.AddTimer(expired_func,data,expired_ms, flag);

}

int deltimer(TimerElem *timer_elem)
{
    return co_centor.time_manager.DelTimer(timer_elem);
}
